#include "Ball.h"
#include <cmath>
#include <algorithm>
#include "Texture.h"

static float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// 정적 멤버 초기화
// 3개 공 텍스처와 현재 선택된 텍스처 ID를 초기화한다.
GLuint Ball::textures[3] = { 0, 0, 0 };
GLuint Ball::currentTexture = 0;

// 텍스처 로딩: 세 가지 공 텍스처를 로드한다. 파일이 없으면 0이 유지된다.
void Ball::LoadTextures() {
    // 기본 공 텍스처: ball.jpg
    textures[0] = Texture::Load("textures/ball.jpg");
    // 추가 공 텍스처: ball1.jpg, ball2.jpg. 존재하지 않을 경우 메시지만 출력하고 0을 유지한다.
    textures[1] = Texture::Load("textures/ball1.jpg");
    textures[2] = Texture::Load("textures/ball2.jpg");
    // 초기값: 첫 번째 텍스처 사용
    currentTexture = textures[0];
}

Ball::Ball() {
    radius = BALL_RADIUS;
    mass = BALL_MASS;
    ballType = 0;

    rollTime = 0.0f;
    totalTime = 0.0f;
    useSpline = false;
    splineSpeed = 0.0f;

    pathStartX = pathEndX = 0.0f;
    pathStartZ = pathEndZ = 0.0f;
    pathAmp = 0.0f;

    Reset();
}

void Ball::Reset() {
    position = vec3(0.0f, radius, 0.5f);
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
    splineSpeed = 0.0f;

    pathStartX = pathEndX = 0.0f;
    pathStartZ = pathEndZ = 0.0f;
    pathAmp = 0.0f;
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
    isInGutter = false;

    spinType = spin;
    rollTime = 0.0f;

    float p = Clamp01(power);
    float shaped = powf(p, 1.15f);

    // ✅ "정상 훅 느낌" 유지: 훅은 속도 상한을 낮게 잡아 순간이동 방지
    float baseSpeed = 7.5f + shaped * 11.0f;  // 대략 7.5 ~ 18.5

    // ✅ 너 요구: "직진만 조금 더 세지면 됨"
    if (spin == SpinType::STRAIGHT) {
        baseSpeed *= 1.25f; // 직진만 25% 상향 (1.15~1.35 조절 가능)
    }
    else {
        // 훅은 너무 빨라지면 totalTime이 너무 작아져서 한 프레임에 끝으로 점프할 수 있음
        if (baseSpeed > 14.5f) baseSpeed = 14.5f;
    }

    splineSpeed = baseSpeed;
    velocity = vec3(0.0f, 0.0f, -baseSpeed);

    if (spin != SpinType::STRAIGHT) {
        useSpline = true;
        SetupSpline(baseSpeed, spin);
    }
    else {
        useSpline = false;
        angularVelocity = vec3(0.0f);
        pathAmp = 0.0f;
    }
}

void Ball::SetupSpline(float baseSpeed, SpinType spin) {
    // ✅ "정상이던" 방식: 고정된 S-curve (시작 위치에 따라 경로도 같이 이동)
    // - Left: 처음 오른쪽(+)
    // - Right: 처음 왼쪽(-)
    // - 끝에서는 startX로 복귀 (중앙에서 던지면 중앙으로 귀결)
    // - 가장자리에서 바깥으로 가면 gutter가 나야 정상 → 여기서는 x 클램프 안 함

    pathStartX = position.x;
    pathStartZ = position.z;

    pathEndX = pathStartX;
    pathEndZ = PIN_START_Z;

    float safeHalf = (LANE_WIDTH * 0.5f) - BALL_RADIUS - 0.02f;
    if (safeHalf < 0.05f) safeHalf = 0.05f;

    float amp = safeHalf * 0.85f;
    float sign = (spin == SpinType::LEFT_HOOK) ? +1.0f : -1.0f;
    pathAmp = sign * amp;

    // 경로 길이 샘플링해서 totalTime 계산 (속도 일정하게 보이게)
    auto evalAt = [&](float t) -> vec3 {
        const float PI = 3.14159265f;
        float x = pathStartX + pathAmp * sinf(PI * t);
        float z = (1.0f - t) * pathStartZ + t * pathEndZ;
        return vec3(x, radius, z);
        };

    float lengthSum = 0.0f;
    vec3 prev = evalAt(0.0f);
    const int N = 80;
    for (int i = 1; i <= N; i++) {
        float t = (float)i / (float)N;
        vec3 cur = evalAt(t);
        lengthSum += length(cur - prev);
        prev = cur;
    }

    float distZ = fabsf(pathEndZ - pathStartZ);
    if (distZ < 0.001f) distZ = 0.001f;
    if (lengthSum < 0.001f) lengthSum = distZ;

    totalTime = lengthSum / std::max(0.1f, baseSpeed);

    // ✅ 순간이동 방지: totalTime이 너무 작으면(한 프레임 내 종료) 최소 시간 보장
    if (totalTime < 0.60f) totalTime = 0.60f;

    angularVelocity = vec3(0.0f, (spin == SpinType::LEFT_HOOK) ? 1.0f : -1.0f, 0.0f);
}

vec3 Ball::EvaluateCardinalSpline(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) {
    // 호환용
    const float tension = 0.5f;
    float t2 = t * t;
    float t3 = t2 * t;

    float b0 = -tension * t3 + 2.0f * tension * t2 - tension * t;
    float b1 = (2.0f - tension) * t3 + (tension - 3.0f) * t2 + 1.0f;
    float b2 = (tension - 2.0f) * t3 + (3.0f - 2.0f * tension) * t2 + tension * t;
    float b3 = tension * t3 - tension * t2;

    return p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;
}

vec3 Ball::EvaluateSpline(float t) {
    t = Clamp01(t);
    const float PI = 3.14159265f;

    float x = pathStartX + pathAmp * sinf(PI * t);
    float z = (1.0f - t) * pathStartZ + t * pathEndZ;

    return vec3(x, radius, z);
}

void Ball::Update(float dt) {
    if (!isRolling) return;

    rollTime += dt;

    if (useSpline && totalTime > 0.0f) {
        float t = rollTime / totalTime;

        if (t >= 1.0f) {
            // 끝점 스냅은 한 번만, 이후 직진
            t = 1.0f;
            position = EvaluateSpline(t);
            useSpline = false;

            float keep = (splineSpeed > 0.1f) ? splineSpeed : 10.0f;
            velocity = vec3(0.0f, 0.0f, -keep);
        }
        else {
            // dt로 속도 뽑지 않고, 접선 기반으로 일정속도 유지 (급가속/순간이동 방지)
            vec3 pos = EvaluateSpline(t);

            float eps = 0.0025f;
            float t2 = t + eps;
            if (t2 > 1.0f) t2 = 1.0f;
            vec3 pos2 = EvaluateSpline(t2);

            vec3 tangent = pos2 - pos;
            if (length(tangent) > 0.00001f) {
                velocity = normalize(tangent) * splineSpeed;
            }

            position = pos;
        }

        position.y = radius;
    }
    else {
        ApplyFriction(dt);
        position += velocity * dt;
    }

    if (position.y < radius) position.y = radius;

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

    // 레인 끝 지나가면 감속/정지
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

    // ✅ 직진만 핀까지 가게: 마찰만 약화 (훅은 건드리지 않음)
    if (spinType == SpinType::STRAIGHT) {
        frictionCoeff *= 0.45f;   // 0.35~0.55 사이에서 취향 조절
    }

    if (isInGutter) frictionCoeff *= 2.0f;

    vec3 frictionDir = -normalize(velocity);
    vec3 frictionForce = frictionDir * frictionCoeff * mass * fabsf(GRAVITY);
    velocity += frictionForce * dt;

    if (length(velocity) < 0.05f) velocity = vec3(0.0f);
}

void Ball::CheckGutter() {
    float gutterEdge = LANE_WIDTH / 2.0f;

    if (fabsf(position.x) > gutterEdge) {
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
    // 텍스처를 사용하여 공을 렌더링한다. 공의 색상은 텍스처와 곱(modulate)되어 표현된다.
    glDisable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    // 공 색상은 타입에 따라 조금씩 다른 색을 적용하지만, 텍스처가 존재하는 경우에는 색상을 흰색으로 설정하여 텍스처가 그대로 보이도록 한다.
    GLfloat matColor[4];
    GLfloat matAmbient[4];
    if (currentTexture != 0) {
        // 텍스처가 있으면 기본 색을 흰색으로, 환경광은 중간 밝기로 한다
        matColor[0] = matColor[1] = matColor[2] = 1.0f;
        matColor[3] = 1.0f;
        matAmbient[0] = matAmbient[1] = matAmbient[2] = 0.5f;
        matAmbient[3] = 1.0f;
    }
    else {
        // 텍스처가 없으면 타입별 색상 사용
        switch (ballType) {
        case 0:
            matColor[0] = 0.9f; matColor[1] = 0.1f; matColor[2] = 0.1f; matColor[3] = 1.0f;
            break;
        case 1:
            matColor[0] = 0.1f; matColor[1] = 0.3f; matColor[2] = 0.9f; matColor[3] = 1.0f;
            break;
        case 2:
            matColor[0] = 0.1f; matColor[1] = 0.8f; matColor[2] = 0.2f; matColor[3] = 1.0f;
            break;
        default:
            matColor[0] = 0.9f; matColor[1] = 0.1f; matColor[2] = 0.1f; matColor[3] = 1.0f;
            break;
        }
        matAmbient[0] = matColor[0] * 0.3f;
        matAmbient[1] = matColor[1] * 0.3f;
        matAmbient[2] = matColor[2] * 0.3f;
        matAmbient[3] = 1.0f;
    }
    GLfloat matSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat matShininess[] = { 100.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // 텍스처 활성화 및 바인딩
    glEnable(GL_TEXTURE_2D);
    if (currentTexture != 0) {
        Texture::Bind(currentTexture, 0);
    }
    // 텍스처가 재질 색상과 곱해지도록 설정
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);

    gluSphere(quad, radius, 32, 32);

    gluDeleteQuadric(quad);
    if (currentTexture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glEnable(GL_COLOR_MATERIAL);
}

void Ball::SetBallType(int type) {
    if (type < 0) type = 0;
    if (type > 2) type = 2;
    ballType = type;
    // 선택된 타입에 따라 현재 텍스처를 업데이트한다. 만약 해당 텍스처가 로드되지 않았다면 0으로 유지된다.
    currentTexture = textures[type];
}