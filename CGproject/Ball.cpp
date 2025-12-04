#include "Ball.h"

Ball::Ball() {
    radius = BALL_RADIUS;
    mass = BALL_MASS;
    spinType = SpinType::STRAIGHT;
    isRolling = false;
    isInGutter = false;
    rotationAngle = 0.0f;
    rotationAxis = vec3(1.0f, 0.0f, 0.0f);
    textureID = 0;
    color = vec3(0.8f, 0.1f, 0.1f);  // 기본 빨간 공
    ballType = 0;
    
    Reset(0.0f);
}

void Ball::Reset(float startX) {
    position = vec3(startX, radius, 0.5f);  // 파울라인 앞
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    isRolling = false;
    isInGutter = false;
    rollTime = 0.0f;
    startX = 0.0f;
    rotationAngle = 0.0f;
}

void Ball::Update(float dt) {
    if (!isRolling) return;
    
    //경과 시간 업데이트
    rollTime += dt;
    // 스핀 효과 적용
    ApplySpinEffect(dt);
    
    // 마찰 적용
    ApplyFriction(dt);
    
    // 위치 업데이트
    position += velocity * dt;
    
    // 회전 업데이트 (시각적 효과)
    float speed = length(velocity);
    if (speed > 0.01f) {
        // 진행 방향에 수직인 축으로 회전
        rotationAxis = normalize(cross(vec3(0.0f, 1.0f, 0.0f), velocity));
        rotationAngle += (speed / radius) * dt * 180.0f / 3.14159f;
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
    }
    
    // 거터 체크
    CheckGutter();
    
    // 공이 레인 끝을 벗어났는지 체크
    if (position.z < -LANE_LENGTH - 1.0f) {
        isRolling = false;
    }
}

void Ball::Launch(float power, SpinType spin) {
    spinType = spin;
    isRolling = true;
    isInGutter = false;
    
    // 기본 속도 (앞으로)
    float baseSpeed = power * 8.0f;  // 파워에 따라 속도 조절
    velocity = vec3(0.0f, 0.0f, -baseSpeed);
    
    // 스핀에 따른 각속도 설정
    // 스핀에 따른 초기 방향 및 각속도 설정
    // 스핀에 따른 초기 방향 및 각속도 설정
    float spinStrength = 0.3f;   // 회전량 감소
    // 시작 위치 및 시간 기록
    startX = position.x;
    rollTime = 0.0f;

    // 목표점 계산: 레인 중간 지점에서 (현재X + 스핀방향으로 레인폭 1/5)
    float laneWidthFifth = LANE_WIDTH / 5.0f;
    float midZ = -LANE_LENGTH / 2.0f;

    switch (spin) {
    case SpinType::STRAIGHT:
        angularVelocity = vec3(0.0f);
        break;
    case SpinType::LEFT_HOOK:
    {
        // 목표: 현재 X + 오른쪽으로 1/5
        float targetX = startX + laneWidthFifth;
        // 목표점까지의 X 방향 속도 계산
        float timeToMid = abs(midZ / velocity.z);
        velocity.x = (targetX - startX) / timeToMid;
        angularVelocity = vec3(0.0f, 1.0f, 0.0f);  // 훅 방향 저장용
    }
    break;
    case SpinType::RIGHT_HOOK:
    {
        // 목표: 현재 X + 왼쪽으로 1/5
        float targetX = startX - laneWidthFifth;
        float timeToMid = abs(midZ / velocity.z);
        velocity.x = (targetX - startX) / timeToMid;
        angularVelocity = vec3(0.0f, -1.0f, 0.0f);  // 훅 방향 저장용
    }
    break;
    }
}

void Ball::ApplySpinEffect(float dt) {
    // 스핀 없으면 무시
    if (abs(angularVelocity.y) < 0.01f) return;

    // 0.8초 이전: 직진 구간 (훅 없음)
    if (rollTime < 0.8f) return;

    // 0.8초 이후: 훅 구간
    float hookTime = rollTime - 0.8f;
    float hookStrength = hookTime * 3.0f;  // 시간에 따라 훅 강해짐
    hookStrength = fmin(hookStrength, 4.0f);  // 최대값 제한

    // 훅 방향 (angularVelocity.y > 0 이면 Left, 왼쪽으로 휘기)
    float hookDirection = (angularVelocity.y > 0) ? -1.0f : 1.0f;

    // X 방향 가속도 적용 (휘어지는 효과)
    velocity.x += hookDirection * hookStrength * dt;

    // 속도 제한
    float maxSideSpeed = 2.5f;
    velocity.x = fmax(-maxSideSpeed, fmin(maxSideSpeed, velocity.x));
}

void Ball::ApplyFriction(float dt) {
    if (length(velocity) < 0.01f) {
        velocity = vec3(0.0f);
        return;
    }
    
    float frictionCoeff = isInGutter ? FRICTION * 2.0f : FRICTION;
    vec3 frictionForce = -normalize(velocity) * frictionCoeff * mass * abs(GRAVITY);
    velocity += frictionForce / mass * dt;
    
    // 최소 속도 이하면 정지
    if (length(velocity) < 0.1f) {
        velocity = vec3(0.0f);
    }
}

void Ball::CheckGutter() {
    float gutterEdge = LANE_WIDTH / 2.0f;
    
    if (abs(position.x) > gutterEdge) {
        isInGutter = true;
        
        // 거터 안에서 이동 (살짝 아래로)
        if (position.y > radius * 0.7f) {
            position.y -= 0.01f;
        }
        
        // 거터 경계에 맞춤
        float gutterCenter = (gutterEdge + GUTTER_WIDTH / 2.0f) * sign(position.x);
        position.x = gutterCenter;
        
        // 옆으로 가는 속도 제거
        velocity.x = 0.0f;
    }
}

void Ball::Draw() {
    glPushMatrix();
    
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
    
    // 색상 설정
    GLfloat matDiffuse[] = { color.r, color.g, color.b, 1.0f };
    GLfloat matSpecular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat matShininess[] = { 80.0f };
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    
    // 텍스처가 있으면 적용
    if (textureID > 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    
    // 구 그리기
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, 32, 32);
    gluDeleteQuadric(quad);
    
    if (textureID > 0) {
        glDisable(GL_TEXTURE_2D);
    }
    
    glPopMatrix();
}

void Ball::SetBallType(int type) {
    ballType = type % 3;  // 0, 1, 2 순환
    
    switch (ballType) {
        case 0:  // 빨간 공
            color = vec3(0.8f, 0.1f, 0.1f);
            break;
        case 1:  // 파란 공
            color = vec3(0.1f, 0.2f, 0.8f);
            break;
        case 2:  // 녹색 공
            color = vec3(0.1f, 0.7f, 0.2f);
            break;
    }
}

bool Ball::IsStopped() {
    return !isRolling || length(velocity) < 0.05f;
}
