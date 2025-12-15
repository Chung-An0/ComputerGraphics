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
    this->startX = startX;
    rotationAngle = 0.0f;
}

void Ball::Update(float dt) {
    if (!isRolling) return;

    //경과 시간 업데이트
    rollTime += dt;

    // [수정 포인트 2] 스핀 효과 적용
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
    float baseSpeed = power * 10.0f;  // 속도감 약간 상향
    velocity = vec3(0.0f, 0.0f, -baseSpeed);

    startX = position.x;
    rollTime = 0.0f;

    // [수정 포인트 2] 스핀에 따른 물리값 설정 (Ver 1 스타일로 복원)
    switch (spin) {
    case SpinType::STRAIGHT:
        angularVelocity = vec3(0.0f);
        break;
    case SpinType::LEFT_HOOK:
        // 왼쪽으로 휘게 하기 위해: 약간 오른쪽으로 출발 + 강한 왼쪽 스핀
        velocity.x = 0.8f;
        angularVelocity = vec3(0.0f, 5.0f, 0.0f); // Y축 회전 (반시계)
        break;
    case SpinType::RIGHT_HOOK:
        // 오른쪽으로 휘게 하기 위해: 약간 왼쪽으로 출발 + 강한 오른쪽 스핀
        velocity.x = -0.8f;
        angularVelocity = vec3(0.0f, -5.0f, 0.0f); // Y축 회전 (시계)
        break;
    }
}

void Ball::ApplySpinEffect(float dt) {
    // 스핀 없으면 무시
    if (abs(angularVelocity.y) < 0.01f) return;

    // [수정 포인트 2] 훅 타이밍과 강도 조절 (확실하게 휘도록)
    if (rollTime < 0.8f) return; // 0.4초 후부터 훅 발생

    float hookTime = rollTime - 0.4f;
    float hookStrength = hookTime * 2.0f;  // 휘는 힘 강화
    hookStrength = fmin(hookStrength, 8.0f);

    // 훅 방향 (angularVelocity.y > 0 이면 Left, 왼쪽으로 휘기)
    float hookDirection = (angularVelocity.y > 0) ? -1.0f : 1.0f;

    // X 방향 가속도 적용 (휘어지는 효과)
    velocity.x += hookDirection * hookStrength * dt;

    // 속도 제한
    float maxSideSpeed = 4.0f;
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

    // [수정 포인트 1] 공 색상 적용 (Material 설정에 color 변수 반영)
    GLfloat matDiffuse[] = { color.r, color.g, color.b, 1.0f };
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 강한 흰색 반사광
    GLfloat matShininess[] = { 100.0f };                // 좁고 선명한 하이라이트
    // Ambient를 높여서 텍스처가 어두워도 색이 잘 드러나게 함
    GLfloat matAmbient[] = { color.r * 0.4f, color.g * 0.4f, color.b * 0.4f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // 텍스처가 있으면 적용하되, 색상(Color)과 혼합되도록 설정
    if (textureID > 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        // 텍스처가 있어도 기본 색상이 묻어나오도록 흰색(Modulate) 설정
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        // 텍스처가 없으면 순수 색상 사용
        glDisable(GL_TEXTURE_2D);
        glColor3f(color.r, color.g, color.b);
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

// [추가] 그림자 그리기 구현
void Ball::DrawShadow(vec3 lightPos) {
    // 1. 조명과 텍스처 끄기 (그림자는 까만색이어야 함)
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();

    // 2. 그림자 행렬 생성 및 적용
    GLfloat shadowMat[16];

    // 바닥 평면: y = 0.001
    glm::vec4 groundPlane(0.0f, 1.0f, 0.0f, -0.001f);
    glm::vec4 lightPos4(lightPos.x, lightPos.y, lightPos.z, 1.0f);

    // Common.h에 추가한 함수 호출
    MakeShadowMatrix(shadowMat, lightPos4, groundPlane);

    // 행렬을 현재 모델뷰 행렬에 곱함
    glMultMatrixf(shadowMat);

    // 3. 공을 원래 위치에 그리기 (납작하게 변환됨)
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    // 4. 짙은 회색으로 설정
    glColor3f(0.15f, 0.15f, 0.15f);

    // 공 모델 그리기
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, radius, 32, 32);
    gluDeleteQuadric(quad);

    glPopMatrix();

    // 5. 상태 복구
    glEnable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f); // 기본 색상 복구
}

void Ball::SetBallType(int type) {
    ballType = type % 3;  // 0, 1, 2 순환

    // [수정 포인트 1] 색상 값을 명확하게 재설정
    switch (ballType) {
    case 0:  // 빨간 공
        color = vec3(0.9f, 0.1f, 0.1f);
        break;
    case 1:  // 파란 공
        color = vec3(0.1f, 0.3f, 0.9f);
        break;
    case 2:  // 녹색 공
        color = vec3(0.1f, 0.8f, 0.2f);
        break;
    }
}

bool Ball::IsStopped() {
    return !isRolling || length(velocity) < 0.05f;
}