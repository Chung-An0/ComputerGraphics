#include "Ball.h"

Ball::Ball() {
    radius = BALL_RADIUS;
    mass = BALL_MASS;
    ballType = 0;
    rollTime = 0.0f;
    totalTime = 0.0f;
    useSpline = false;
    Reset();
}

void Ball::Reset() {
    position = vec3(0.0f, radius, 0.5f);  // 파울라인(z=0) 바로 뒤
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotationAngle = 0.0f;
    rotationAxis = vec3(1.0f, 0.0f, 0.0f);
    isRolling = false;
    isInGutter = false;
    spinType = SpinType::STRAIGHT;
    rollTime = 0.0f;
    totalTime = 0.0f;
    useSpline = false;
    splineP0 = vec3(0.0f);
    splineP1 = vec3(0.0f);
    splineP2 = vec3(0.0f);
    splineP3 = vec3(0.0f);
}

void Ball::Reset(float startX) {
    Reset();
    position.x = startX;
}

bool Ball::IsStopped() {
    return !isRolling || length(velocity) < 0.1f;
}

void Ball::Launch(float power, SpinType spin) {
    isRolling = true;
    spinType = spin;
    rollTime = 0.0f;
    isInGutter = false;

    // 기본 전진 속도 (파워에 비례) - 속도 증가
    float baseSpeed = power * 12.0f + 5.0f;
    velocity = vec3(0.0f, 0.0f, -baseSpeed);

    // 스플라인 설정 (스핀이 있을 때만)
    if (spin != SpinType::STRAIGHT) {
        useSpline = true;
        SetupSpline(baseSpeed, spin);
    }
    else {
        useSpline = false;
        angularVelocity = vec3(0.0f);
    }
}

void Ball::SetupSpline(float baseSpeed, SpinType spin) {
    // 핀까지 거리
    float distanceToPins = abs(PIN_START_Z - position.z);

    // 전체 이동 시간 (거리 / 속도)
    totalTime = distanceToPins / baseSpeed;

    // 레인 폭의 1/5
    float laneOffset = LANE_WIDTH / 5.0f;

    // 0.8초 시점이 전체의 몇 %인지
    float hookStartRatio = 0.8f / totalTime;
    if (hookStartRatio > 0.4f) hookStartRatio = 0.4f;  // 최대 40% 지점

    // 중간 지점 Z 좌표
    float midZ = position.z - distanceToPins * hookStartRatio;

    // 제어점 설정
    splineP1 = position;  // 시작점

    if (spin == SpinType::LEFT_HOOK) {
        // P0: 시작점 뒤 (방향 결정용)
        splineP0 = position + vec3(-laneOffset * 0.3f, 0.0f, 0.5f);
        // P2: 중간점 - 오른쪽으로 1/5 이동
        splineP2 = vec3(position.x + laneOffset, radius, midZ);
        // P3: 끝점 - 가운데로 돌아옴 (핀 위치)
        splineP3 = vec3(position.x - laneOffset * 0.3f, radius, PIN_START_Z);
    }
    else {  // RIGHT_HOOK
        splineP0 = position + vec3(laneOffset * 0.3f, 0.0f, 0.5f);
        splineP2 = vec3(position.x - laneOffset, radius, midZ);
        splineP3 = vec3(position.x + laneOffset * 0.3f, radius, PIN_START_Z);
    }

    // 회전 방향 저장
    angularVelocity = vec3(0.0f, (spin == SpinType::LEFT_HOOK) ? 1.0f : -1.0f, 0.0f);
}

vec3 Ball::EvaluateCardinalSpline(float t) {
    // Cardinal Spline (Catmull-Rom) 공식
    // tension = 0.5 (기본값)
    float tension = 0.5f;

    float t2 = t * t;
    float t3 = t2 * t;

    // 기반 함수 (Basis functions)
    float b0 = -tension * t3 + 2.0f * tension * t2 - tension * t;
    float b1 = (2.0f - tension) * t3 + (tension - 3.0f) * t2 + 1.0f;
    float b2 = (tension - 2.0f) * t3 + (3.0f - 2.0f * tension) * t2 + tension * t;
    float b3 = tension * t3 - tension * t2;

    // 보간된 위치
    vec3 result = splineP0 * b0 + splineP1 * b1 + splineP2 * b2 + splineP3 * b3;
    return result;
}

void Ball::Update(float dt) {
    if (!isRolling) return;

    rollTime += dt;

    if (useSpline && totalTime > 0.0f) {
        // 스플라인 기반 이동
        float t = rollTime / totalTime;

        if (t >= 1.0f) {
            // 스플라인 끝 - 직진으로 전환
            t = 1.0f;
            useSpline = false;
            position = splineP3;
            position.y = radius;

            // 현재 속도 유지하면서 직진
            float currentSpeed = length(velocity);
            if (currentSpeed < 5.0f) currentSpeed = 5.0f;
            velocity = vec3(0.0f, 0.0f, -currentSpeed);
        }
        else {
            vec3 newPos = EvaluateCardinalSpline(t);

            // 속도 계산 (위치 변화로부터)
            if (dt > 0.0001f) {
                velocity = (newPos - position) / dt;
            }
            position = newPos;
            position.y = radius;
        }
    }
    else {
        // 일반 물리 기반 이동
        ApplyFriction(dt);
        position += velocity * dt;
    }

    // 높이 보정
    if (position.y < radius) {
        position.y = radius;
    }

    // 거터 체크
    CheckGutter();

    // 회전 애니메이션
    float speed = length(velocity);
    if (speed > 0.01f) {
        rotationAngle += speed * dt * 100.0f;
        vec3 cross_result = cross(vec3(0, 1, 0), velocity);
        if (length(cross_result) > 0.001f) {
            rotationAxis = normalize(cross_result);
        }
    }

    // 레인 끝 도달 또는 정지 체크
    if (position.z < PIN_START_Z - 2.0f || length(velocity) < 0.1f) {
        velocity *= 0.8f;
        if (length(velocity) < 0.05f) {
            isRolling = false;
            velocity = vec3(0.0f);
        }
    }
}

void Ball::ApplyFriction(float dt) {
    float speed = length(velocity);
    if (speed < 0.01f) return;

    float frictionCoeff = FRICTION;
    if (isInGutter) {
        frictionCoeff *= 2.0f;
    }

    vec3 frictionDir = -normalize(velocity);
    vec3 frictionForce = frictionDir * frictionCoeff * mass * abs(GRAVITY);
    velocity += frictionForce * dt;

    if (length(velocity) < 0.05f) {
        velocity = vec3(0.0f);
    }
}

void Ball::CheckGutter() {
    float gutterEdge = LANE_WIDTH / 2.0f;

    if (abs(position.x) > gutterEdge) {
        isInGutter = true;

        if (position.x > gutterEdge) {
            position.x = gutterEdge + GUTTER_WIDTH / 2.0f;
        }
        else {
            position.x = -gutterEdge - GUTTER_WIDTH / 2.0f;
        }

        velocity.x = 0.0f;
        useSpline = false;
    }
}

void Ball::Draw() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);  // Material 설정이 제대로 적용되도록

    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    // 공 색상 (ballType에 따라)
    GLfloat matColor[4];

    switch (ballType) {
    case 0:  // 빨강
        matColor[0] = 0.9f; matColor[1] = 0.1f; matColor[2] = 0.1f; matColor[3] = 1.0f;
        break;
    case 1:  // 파랑
        matColor[0] = 0.1f; matColor[1] = 0.3f; matColor[2] = 0.9f; matColor[3] = 1.0f;
        break;
    case 2:  // 초록
        matColor[0] = 0.1f; matColor[1] = 0.8f; matColor[2] = 0.2f; matColor[3] = 1.0f;
        break;
    default:
        matColor[0] = 0.9f; matColor[1] = 0.1f; matColor[2] = 0.1f; matColor[3] = 1.0f;
        break;
    }

    GLfloat matAmbient[] = { matColor[0] * 0.3f, matColor[1] * 0.3f, matColor[2] * 0.3f, 1.0f };
    GLfloat matSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat matShininess[] = { 100.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, radius, 32, 32);
    gluDeleteQuadric(quad);

    glPopMatrix();

    glEnable(GL_COLOR_MATERIAL);  // 다른 오브젝트를 위해 다시 활성화
}

void Ball::SetBallType(int type) {
    if (type < 0) type = 0;
    if (type > 2) type = 2;
    ballType = type;
}