#include "Pin.h"

// 핀 배치 위치 (삼각형)
//        7  8  9  10
//          4  5  6
//            2  3
//              1
static vec3 PIN_POSITIONS[10] = {
    vec3(0.0f, 0.0f, 0.0f),                          // 1번 (헤드 핀)
    vec3(-PIN_SPACING * 0.5f, 0.0f, -PIN_SPACING),   // 2번
    vec3(PIN_SPACING * 0.5f, 0.0f, -PIN_SPACING),    // 3번
    vec3(-PIN_SPACING, 0.0f, -PIN_SPACING * 2),      // 4번
    vec3(0.0f, 0.0f, -PIN_SPACING * 2),              // 5번
    vec3(PIN_SPACING, 0.0f, -PIN_SPACING * 2),       // 6번
    vec3(-PIN_SPACING * 1.5f, 0.0f, -PIN_SPACING * 3), // 7번
    vec3(-PIN_SPACING * 0.5f, 0.0f, -PIN_SPACING * 3), // 8번
    vec3(PIN_SPACING * 0.5f, 0.0f, -PIN_SPACING * 3),  // 9번
    vec3(PIN_SPACING * 1.5f, 0.0f, -PIN_SPACING * 3)   // 10번
};

Pin::Pin() {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;
    mass = 1.5f;  // 약 1.5kg
    pinNumber = 0;
    Reset();
}

Pin::Pin(int number, vec3 pos) {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;
    mass = 1.5f;
    pinNumber = number;
    position = pos;
    position.y = height / 2.0f;  // 핀 중심 높이
    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);
}

void Pin::Reset() {
    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);
    position.y = height / 2.0f;
}

void Pin::Update(float dt) {
    if (isStanding && !isFalling) return;

    // 중력 적용
    velocity.y += GRAVITY * dt;

    // 위치 업데이트
    position += velocity * dt;

    // 회전 업데이트
    rotation += angularVelocity * dt * 50.0f;

    // 마찰로 감속
    velocity *= 0.98f;
    angularVelocity *= 0.95f;

    // 바닥 충돌
    CheckFloor();

    // 쓰러짐 판정
    if (abs(rotation.x) > 45.0f || abs(rotation.z) > 45.0f) {
        isStanding = false;
    }
}

bool Pin::CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3 ballVelocity, vec3 ballAngularVelocity) {
    if (!isStanding) return false;

    // 실린더-구 충돌 (충돌 범위 확대)
    vec3 toPin = position - ballPos;
    toPin.y = 0;  // 수평 거리만

    float dist = length(toPin);
    float collisionDist = (radius + ballRadius) * 1.3f;  // 충돌 범위 30% 확대

    if (dist < collisionDist) {
        // 충돌!
        isFalling = true;

        vec3 impactDir = normalize(toPin);
        float impactForce = length(ballVelocity) * 3.0f;//충격력 증가

        // 기본 충격 적용
        velocity = impactDir * impactForce * 0.8f;

        // 공의 회전(스핀)에 따른 추가 힘
        vec3 spinEffect = cross(ballAngularVelocity, impactDir) * 0.4f;
        velocity += spinEffect;

        velocity.y = impactForce * 0.4f;  // 위로 튀어오름

        // 회전 적용 (공의 진행 방향과 스핀 기반)
        vec3 ballDir = normalize(ballVelocity);
        angularVelocity.x = -impactDir.z * impactForce * 2.0f + ballAngularVelocity.y * 0.5f;
        angularVelocity.y = (float)(rand() % 60 - 30) / 10.0f;
        angularVelocity.z = impactDir.x * impactForce * 2.0f - ballAngularVelocity.y * 0.5f;

        return true;
    }

    return false;
}

bool Pin::CheckCollisionWithPin(Pin& other) {
    if (!other.isFalling && !other.isStanding) return false;
    if (this == &other) return false;

    vec3 diff = other.position - position;
    float dist = length(diff);
    float collisionDist = radius * 2.8f;  // 약간 넓게

    if (dist < collisionDist && dist > 0.001f) {
        vec3 normal = normalize(diff);

        // 위치 보정
        float overlap = collisionDist - dist;
        position -= normal * overlap * 0.5f;
        other.position += normal * overlap * 0.5f;

        // 속도 교환 (탄성 충돌)
        vec3 relVel = velocity - other.velocity;
        float velAlongNormal = dot(relVel, normal);

        if (velAlongNormal > 0) return false;

        float restitution = 0.8f;
        float impulse = -(1 + restitution) * velAlongNormal / 2.0f;

        vec3 impulseVec = impulse * normal;
        velocity += impulseVec;
        other.velocity -= impulseVec;

        // 쓰러지는 핀이 서있는 핀을 칠 때
        if (isFalling && other.isStanding) {
            float hitForce = length(velocity);
            if (hitForce > 0.2f) {  // 더 쉽게 쓰러짐
                other.isStanding = false;
                other.isFalling = true;
                other.velocity = normal * hitForce * 0.6f;
                other.velocity.y = 0.8f;

                // 맞은 방향으로 회전
                other.angularVelocity = vec3(
                    normal.z * hitForce * 2.0f,
                    (float)(rand() % 40 - 20) / 10.0f,
                    -normal.x * hitForce * 2.0f
                );
            }
        }

        // 서있는 핀이 쓰러지는 핀에게 맞을 때
        if (isStanding && other.isFalling) {
            float hitForce = length(other.velocity);
            if (hitForce > 0.5f) {
                isStanding = false;
                isFalling = true;
                velocity = -normal * hitForce * 0.6f;
                velocity.y = 0.8f;

                angularVelocity = vec3(
                    -normal.z * hitForce * 2.0f,
                    (float)(rand() % 40 - 20) / 10.0f,
                    normal.x * hitForce * 2.0f
                );
            }
        }

        // 둘 다 쓰러지는 중일 때
        if (isFalling && other.isFalling) {
            // 각속도도 일부 전달
            vec3 avgAngVel = (angularVelocity + other.angularVelocity) * 0.5f;
            angularVelocity = avgAngVel + vec3((rand() % 20 - 10) / 10.0f, 0, (rand() % 20 - 10) / 10.0f);
            other.angularVelocity = avgAngVel + vec3((rand() % 20 - 10) / 10.0f, 0, (rand() % 20 - 10) / 10.0f);
        }

        return true;
    }

    return false;
}

void Pin::ApplyImpact(vec3 impactDir, float force) {
    isFalling = true;

    // 속도 적용
    velocity = impactDir * force * 0.8f;
    velocity.y = force * 0.3f;  // 살짝 위로

    // 회전 적용 (충돌 방향 반대로 쓰러짐)
    angularVelocity.x = -impactDir.z * force * 2.0f;
    angularVelocity.z = impactDir.x * force * 2.0f;
}

void Pin::CheckFloor() {
    // 바닥 아래로 안 가게
    if (position.y < height / 2.0f) {
        position.y = height / 2.0f;
        velocity.y = 0;

        // 바닥에서 마찰
        velocity.x *= 0.9f;
        velocity.z *= 0.9f;
    }

    // 완전히 쓰러진 상태
    if (IsDown()) {
        velocity = vec3(0.0f);
        angularVelocity = vec3(0.0f);
    }
}

bool Pin::IsDown() {
    return !isStanding || abs(rotation.x) > 80.0f || abs(rotation.z) > 80.0f;
}

void Pin::Draw() {
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);

    // 핀 색상 (흰색 + 빨간 줄무늬)
    GLfloat matWhite[] = { 0.95f, 0.95f, 0.95f, 1.0f };
    GLfloat matRed[] = { 0.8f, 0.1f, 0.1f, 1.0f };
    GLfloat matSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat matShininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // 핀 몸통 (원기둥 + 구로 근사)
    GLUquadric* quad = gluNewQuadric();

    // 아래 부분 (굵은 원기둥)
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matWhite);
    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f + 0.02f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius * 1.2f, radius, height * 0.3f, 16, 4);
    glPopMatrix();

    // 중간 부분 (좁아지는)
    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f + height * 0.3f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius, radius * 0.6f, height * 0.4f, 16, 4);
    glPopMatrix();

    // 목 부분 (빨간 줄무늬)
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matRed);
    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f + height * 0.65f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius * 0.6f, radius * 0.5f, height * 0.1f, 16, 2);
    glPopMatrix();

    // 머리 부분
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matWhite);
    glPushMatrix();
    glTranslatef(0.0f, height / 2.0f - radius * 0.8f, 0.0f);
    gluSphere(quad, radius * 0.7f, 16, 16);
    glPopMatrix();

    gluDeleteQuadric(quad);

    glPopMatrix();
}

// ============ PinManager ============

PinManager::PinManager() {
    ResetAll();
}

void PinManager::ResetAll() {
    for (int i = 0; i < 10; i++) {
        vec3 pos = PIN_POSITIONS[i];
        pos.z += PIN_START_Z;  // 레인 끝쪽으로 이동
        pins[i] = Pin(i + 1, pos);
    }
    standingCount = 10;
}

void PinManager::RemoveFallenPins() {
    // 쓰러진 핀은 그대로 두고, 서있는 핀만 리셋
    for (int i = 0; i < 10; i++) {
        if (pins[i].isStanding && !pins[i].IsDown()) {
            pins[i].velocity = vec3(0.0f);
            pins[i].angularVelocity = vec3(0.0f);
        }
    }
}

void PinManager::Update(float dt) {
    for (int i = 0; i < 10; i++) {
        pins[i].Update(dt);
    }
    CheckPinCollisions();
    standingCount = CountStanding();
}

void PinManager::CheckBallCollision(vec3 ballPos, float ballRadius, vec3 ballVelocity, vec3 ballAngularVelocity) {
    for (int i = 0; i < 10; i++) {
        pins[i].CheckCollisionWithBall(ballPos, ballRadius, ballVelocity, ballAngularVelocity);
    }
}

void PinManager::CheckPinCollisions() {
    // 여러 번 반복해서 연쇄 충돌 처리
    for (int iter = 0; iter < 3; iter++) {
        for (int i = 0; i < 10; i++) {
            for (int j = i + 1; j < 10; j++) {
                pins[i].CheckCollisionWithPin(pins[j]);
            }
        }
    }
}

void PinManager::Draw() {
    for (int i = 0; i < 10; i++) {
        pins[i].Draw();
    }
}

int PinManager::CountStanding() {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (pins[i].isStanding && !pins[i].IsDown()) {
            count++;
        }
    }
    return count;
}

bool PinManager::AllSettled() {
    for (int i = 0; i < 10; i++) {
        if (length(pins[i].velocity) > 0.05f) {
            return false;
        }
    }
    return true;
}