#include "Pin.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// static 멤버 변수 초기화
GLuint Pin::pinTextureID = 0;
bool Pin::textureLoaded = false;

// 핀 배치 위치 (삼각형)
static vec3 PIN_POSITIONS[10] = {
    vec3(0.0f, 0.0f, 0.0f),                          // 1번
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


void Pin::InitTexture(const char* filepath) {
    if (textureLoaded) return;

    // Lane과 동일하게 stb_image 기반 로더 사용
    pinTextureID = Texture::Load(filepath);
    textureLoaded = (pinTextureID != 0);
}
// ========== Cardinal Spline 보간 (1D) ==========
float Pin::EvaluateCardinalSpline1D(float p0, float p1, float p2, float p3, float t) {
    const float tension = 0.5f;
    float t2 = t * t;
    float t3 = t2 * t;

    float b0 = -tension * t3 + 2.0f * tension * t2 - tension * t;
    float b1 = (2.0f - tension) * t3 + (tension - 3.0f) * t2 + 1.0f;
    float b2 = (tension - 2.0f) * t3 + (3.0f - 2.0f * tension) * t2 + tension * t;
    float b3 = tension * t3 - tension * t2;

    return p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;
}

// ========== 볼링핀 프로필 곡선 (머리까지 자연스럽게) ==========
vec2 Pin::GetProfilePoint(float t) {
    // t:  0 (바닥) ~ 1 (머리 끝)

    // 9개 제어점으로 부드러운 곡선 생성
    const int numControlPoints = 8;
    float controlY[8] = {
        0.00f,   // 0. 바닥
        0.25f,   // 2. 중간 (허리)
        0.70f,   // 4. 목
        0.85f,   // 5. 머리 시작
        0.90f,
        0.95f,   // 7. 머리 상단 (점점 작아짐)
        0.98f,
        1.00f    // 8. 머리 끝 (뾰족하게 닫힘)
    };

    float controlRadius[8] = {
        0.028f,  // 0. 바닥 반지름 (넓음)
        0.056f,  // 2. 중간 (가늘어짐)
        0.020f,  // 4. 목 (가장 가늘음)
        0.023f,  // 5. 머리 시작
        0.018f,
        0.012f,  // 7. 머리 상단 (작아짐)
        0.002f,
        0.001f   // 8. ✅ 머리 끝 (거의 0 = 닫힘)
    };

    // t에 해당하는 구간 찾기
    int segment = 0;
    for (int i = 0; i < numControlPoints - 1; i++) {
        if (t >= controlY[i] && t <= controlY[i + 1]) {
            segment = i;
            break;
        }
    }

    // 경계 처리
    if (segment == 0 && t < controlY[0]) segment = 0;
    if (segment >= numControlPoints - 2) segment = numControlPoints - 3;

    // Cardinal Spline 보간을 위한 4개 점
    int i0 = std::max(0, segment - 1);
    int i1 = segment;
    int i2 = std::min(numControlPoints - 1, segment + 1);
    int i3 = std::min(numControlPoints - 1, segment + 2);

    // 로컬 파라미터 계산
    float localT = 0.0f;
    if (controlY[i2] - controlY[i1] > 0.0001f) {
        localT = (t - controlY[i1]) / (controlY[i2] - controlY[i1]);
    }
    localT = std::max(0.0f, std::min(1.0f, localT));

    // Spline 보간
    float radius = EvaluateCardinalSpline1D(
        controlRadius[i0],
        controlRadius[i1],
        controlRadius[i2],
        controlRadius[i3],
        localT
    );

    // 음수 방지 (머리 끝에서)
    if (radius < 0.001f) radius = 0.001f;

    float y = t * height;

    return vec2(radius, y);
}

// ========== 회전체 메쉬 생성 ==========
void Pin::GenerateRevolutionMesh() {
    if (meshGenerated) return;

    vertices.clear();
    indices.clear();

    const int heightSegments = 30;  // 높이 방향 세그먼트 (부드러운 곡선)
    const int radialSegments = 32;  // 회전 방향 세그먼트 (원형)

    // ========== Vertex 생성 ==========
    for (int i = 0; i <= heightSegments; i++) {
        float t = (float)i / (float)heightSegments;

        // t를 0.0~0.90 범위로 매핑 (머리 15%는 구로 대체)
        t = t * 0.90f;

        vec2 profile = GetProfilePoint(t);
        float r = profile.x;
        float y = profile.y - height / 2.0f; // 중심을 원점으로

        // Normal 계산용 접선
        float eps = 0.01f;
        vec2 profileNext = GetProfilePoint(std::min(1.0f, t + eps));
        vec2 tangent2D = normalize(profileNext - profile);

        for (int j = 0; j <= radialSegments; j++) {
            float angle = (float)j / (float)radialSegments * 2.0f * 3.14159265f;

            Vertex v;
            v.position = vec3(
                r * cos(angle),
                y,
                r * sin(angle)
            );

            // Normal 계산 (회전체의 경우)
            vec3 radialDir = normalize(vec3(cos(angle), 0.0f, sin(angle)));
            vec3 heightDir = vec3(0.0f, tangent2D.y, 0.0f);
            vec3 normal = normalize(radialDir - heightDir * tangent2D.x);
            v.normal = normal;

            // ========== UV 좌표 계산==========
            v.texCoord = vec2(
                (float)j / (float)radialSegments,  // u:  0~1
                1.0f - (t / 0.90f)                 // v: 1~0
            );

            vertices.push_back(v);
        }
    }

    // ========== Index 생성 (삼각형) ==========
    for (int i = 0; i < heightSegments; i++) {
        for (int j = 0; j < radialSegments; j++) {
            int current = i * (radialSegments + 1) + j;
            int next = current + radialSegments + 1;

            // 사각형 → 2개 삼각형
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    meshGenerated = true;
}

// ========== 생성자 ==========
Pin::Pin() {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;
    mass = 1.53f;
    pinNumber = 0;
    meshGenerated = false;

    // 무게 중심 (하단 1/3 지점)
    centerOfMass = vec3(0.0f, height * 0.20f, 0.0f);
    // 관성 모멘트
    inertia = mass * radius * radius * 0.5f;

    Reset();
    GenerateRevolutionMesh();
}

Pin::Pin(int number, vec3 pos) {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;
    mass = 1.5f;
    pinNumber = number;
    position = pos;
    position.y = height / 2.0f;

    centerOfMass = vec3(0.0f, height * 0.35f, 0.0f);
    inertia = mass * radius * radius * 0.4f;

    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);
    meshGenerated = false;
    GenerateRevolutionMesh();
}

void Pin::Reset() {
    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);
    position.y = height / 2.0f;
}

// ========== 물리 업데이트 ==========
void Pin::Update(float dt) {
    if (isStanding && !isFalling) return;

    // 중력 적용
    velocity.y += GRAVITY * dt;
    position += velocity * dt;

    // 회전 업데이트
    rotation += angularVelocity * dt * 57.2958f; // 라디안 -> 도

    // 공기 저항 및 마찰 (좀 더 강하게)
    velocity *= 0.97f;
    angularVelocity *= 0.88f;

    CheckFloor();

    // 쓰러짐 판정 (35도 이상 - 더 민감하게)
    if (abs(rotation.x) > 35.0f || abs(rotation.z) > 35.0f) {
        isStanding = false;
    }

    // 완전히 쓰러진 후 안정화
    if (IsDown() && length(velocity) < 0.1f) {
        velocity *= 0.9f;
        angularVelocity *= 0.85f;

        if (length(velocity) < 0.05f && length(angularVelocity) < 0.05f) {
            velocity = vec3(0.0f);
            angularVelocity = vec3(0.0f);
        }
    }
}

bool Pin::CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity) {
    vec3 toPin = position - ballPos;
    float dist = length(toPin);
    float collisionDist = (radius + ballRadius) * 1.15f;

    if (dist < collisionDist && dist > 0.001f) {
        vec3 impactDir = normalize(toPin);
        float ballSpeed = length(ballVelocity);

        // ✅ 볼링공 질량 증가 (7. 25kg)
        float ballMass = 7.25f;
        float totalMass = ballMass + mass;

        // ✅ 서 있는 핀과 쓰러진 핀 구분
        if (isStanding) {
            // 서 있는 핀:  정상적으로 쓰러뜨리기
            isFalling = true;
            isStanding = false;

            float impactHeight = ballPos.y - (position.y - height / 2.0f);
            impactHeight = std::max(0.0f, std::min(height, impactHeight));
            float heightRatio = impactHeight / height;

            float restitution = 0.4f;
            vec3 ballVelNormal = dot(ballVelocity, impactDir) * impactDir;
            float ballVelMag = length(ballVelNormal);

            float pinSpeed = (2.0f * ballMass * ballVelMag) / totalMass;

            velocity = impactDir * pinSpeed * 0.55f;
            velocity.y = pinSpeed * 0.12f * (0.8f + heightRatio * 0.4f);

            vec3 spinEffect = cross(ballAngularVelocity, impactDir) * 0.15f;
            velocity += spinEffect;

            vec3 leverArm = vec3(0.0f, impactHeight - centerOfMass.y, 0.0f);
            vec3 impactForce = impactDir * pinSpeed;
            vec3 torque = cross(leverArm, impactForce);

            angularVelocity = torque / inertia * 0.8f;
            angularVelocity.x += -impactDir.z * pinSpeed * 0.9f;
            angularVelocity.z += impactDir.x * pinSpeed * 0.9f;
            angularVelocity.y = (float)(rand() % 20 - 10) / 30.0f;

            float ballSpeedLoss = (mass * pinSpeed) / ballMass;
            ballVelocity -= impactDir * ballSpeedLoss * (1.0f + restitution);

            if (length(ballVelocity) < ballSpeed * 0.3f) {
                ballVelocity = normalize(ballVelocity) * ballSpeed * 0.3f;
            }
        }
        else {
            // ✅ 쓰러진 핀:  가볍게 밀려남 (공은 거의 영향 안 받음)
            float restitution = 0.15f;  // 매우 낮은 반발

            vec3 relativeVel = ballVelocity - velocity;
            float velAlongNormal = dot(relativeVel, impactDir);

            if (velAlongNormal < 0) return false;

            // ✅ 질량 비율 (핀이 훨씬 가벼움:  1.53 vs 7.25)
            float massRatio = mass / ballMass;  // 약 0.21

            // ✅ 핀이 받는 충격 (약하게)
            float impulse = (1.0f + restitution) * velAlongNormal / (1.0f + massRatio);

            // ✅ 핀은 수평으로 밀려남
            vec3 pushDir = impactDir;
            pushDir.y = 0;
            if (length(pushDir) > 0.001f) {
                pushDir = normalize(pushDir);

                // 수평 방향으로만 밀림
                velocity.x += pushDir.x * impulse * massRatio * 0.4f;
                velocity.z += pushDir.z * impulse * massRatio * 0.4f;
                velocity.y += impulse * massRatio * 0.02f;  // 거의 안 튀어오름

                // 약간의 회전
                angularVelocity += vec3(
                    pushDir.z * impulse * massRatio * 0.2f,
                    0.0f,
                    -pushDir.x * impulse * massRatio * 0.2f
                );
            }

            // 볼링공은 거의 영향 안 받음 (질량이 훨씬 무거우니까)
            ballVelocity -= impactDir * impulse * 0.08f;
        }

        return true;
    }

    return false;
}

// ========== 핀-핀 충돌 (개선) ==========
bool Pin::CheckCollisionWithPin(Pin& other) {
    // ✅ 둘 다 완전히 멈춘 상태면 충돌 체크 안 함
    if (!isFalling && !isStanding && !other.isFalling && !other.isStanding) {
        if (length(velocity) < 0.01f && length(other.velocity) < 0.01f) {
            return false;
        }
    }

    if (this == &other) return false;

    vec3 diff = other.position - position;
    diff.y = 0;  // ✅ 수평 거리만 체크
    float dist = length(diff);
    float collisionDist = radius * 2.3f;  // ✅ 충돌 거리 조정

    if (dist < collisionDist && dist > 0.001f) {
        vec3 normal = normalize(diff);

        // ✅ 침투 깊이 해결
        float overlap = collisionDist - dist;
        position -= normal * overlap * 0.5f;
        other.position += normal * overlap * 0.5f;

        // ✅ 상대 속도 계산 (수평만)
        vec3 relVel = velocity - other.velocity;
        relVel.y = 0;
        float velAlongNormal = dot(relVel, normal);

        if (velAlongNormal > 0) return false;

        // ✅ 충돌 반발력 (낮은 반발)
        float restitution = 0.35f;
        float impulse = -(1 + restitution) * velAlongNormal / 2.0f;

        vec3 impulseVec = impulse * normal;
        velocity.x += impulseVec.x;
        velocity.z += impulseVec.z;
        other.velocity.x -= impulseVec.x;
        other.velocity.z -= impulseVec.z;

        // ✅ 서 있는 핀을 쓰러뜨리기
        if (isFalling && other.isStanding) {
            float hitForce = length(vec3(velocity.x, 0, velocity.z));

            if (hitForce > 0.25f) {  // ✅ 임계값 낮춤
                other.isStanding = false;
                other.isFalling = true;

                float impactHeight = position.y - (other.position.y - other.height / 2.0f);
                impactHeight = std::max(0.0f, std::min(other.height * 0.6f, impactHeight));
                float heightRatio = impactHeight / other.height;

                // ✅ 수평 속도 위주
                other.velocity = normal * hitForce * 0.5f;
                other.velocity.y = hitForce * 0.08f * (1.0f + heightRatio * 0.3f);  // ✅ 수직 속도 감소

                // ✅ 토크 적용
                vec3 leverArm = vec3(0.0f, impactHeight - other.centerOfMass.y, 0.0f);
                vec3 force = normal * hitForce;
                vec3 torque = cross(leverArm, force);

                other.angularVelocity = torque / other.inertia * 0.7f;
                other.angularVelocity.x += normal.z * hitForce * 0.8f;
                other.angularVelocity.z += -normal.x * hitForce * 0.8f;
                other.angularVelocity.y = (float)(rand() % 20 - 10) / 25.0f;
            }
        }

        // ✅ 반대 경우
        if (isStanding && other.isFalling) {
            float hitForce = length(vec3(other.velocity.x, 0, other.velocity.z));

            if (hitForce > 0.25f) {
                isStanding = false;
                isFalling = true;

                float impactHeight = other.position.y - (position.y - height / 2.0f);
                impactHeight = std::max(0.0f, std::min(height * 0.6f, impactHeight));
                float heightRatio = impactHeight / height;

                velocity = -normal * hitForce * 0.5f;
                velocity.y = hitForce * 0.08f * (1.0f + heightRatio * 0.3f);

                vec3 leverArm = vec3(0.0f, impactHeight - centerOfMass.y, 0.0f);
                vec3 force = -normal * hitForce;
                vec3 torque = cross(leverArm, force);

                angularVelocity = torque / inertia * 0.7f;
                angularVelocity.x += -normal.z * hitForce * 0.8f;
                angularVelocity.z += normal.x * hitForce * 0.8f;
                angularVelocity.y = (float)(rand() % 20 - 10) / 25.0f;
            }
        }

        // ✅ 둘 다 쓰러져 있을 때
        if (isFalling && other.isFalling) {
            // ✅ 에너지 감쇠
            velocity *= 0.95f;
            other.velocity *= 0.95f;

            // ✅ 회전 약간 교환
            vec3 avgAngVel = (angularVelocity + other.angularVelocity) * 0.5f;
            angularVelocity = avgAngVel * 0.85f;
            other.angularVelocity = avgAngVel * 0.85f;
        }

        return true;
    }

    return false;
}

void Pin::ApplyImpact(vec3 impactDir, float force) {
    isFalling = true;

    velocity = impactDir * force * 0.8f;
    velocity.y = force * 0.3f;

    angularVelocity.x = -impactDir.z * force * 2.0f;
    angularVelocity.z = impactDir.x * force * 2.0f;
}

void Pin::CheckFloor() {
    // ✅ 회전 각도에 따라 동적으로 바닥 높이 계산
    float rotMag = sqrt(rotation.x * rotation.x + rotation.z * rotation.z);
    float tiltRatio = std::min(1.0f, rotMag / 90.0f);

    float minY;
    if (isStanding) {
        minY = height / 2.0f;
    }
    else {
        // ✅ 0도(서있음) → height/2, 90도(완전 쓰러짐) → radius * 0.6
        minY = (1.0f - tiltRatio) * (height / 2.0f) + tiltRatio * (radius * 0.6f);
    }

    if (position.y < minY) {
        position.y = minY;

        // ✅ 바닥 반발 거의 없음
        if (velocity.y < -0.1f) {
            velocity.y = -velocity.y * 0.10f;
        }
        else {
            velocity.y = 0;
        }

        // ✅ 강한 마찰
        velocity.x *= 0.85f;
        velocity.z *= 0.85f;
        angularVelocity *= 0.82f;
    }

    // ✅ 쓰러진 후 안정화
    if (IsDown()) {
        if (length(velocity) < 0.12f) {
            velocity *= 0.88f;
            angularVelocity *= 0.80f;

            if (length(velocity) < 0.05f) velocity = vec3(0.0f);
            if (length(angularVelocity) < 0.05f) angularVelocity = vec3(0.0f);
        }
    }
}

bool Pin::IsDown() {
    return !isStanding || abs(rotation.x) > 75.0f || abs(rotation.z) > 75.0f;
}

void Pin::Draw() {
    if (!meshGenerated) {
        GenerateRevolutionMesh();
    }

    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);

    const bool useTex = (textureLoaded && pinTextureID != 0);

    if (useTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, pinTextureID);

        // Lane과 동일한 모드 (조명*텍스처)
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        // 텍스처 색이 죽지 않게 재질을 흰색으로
        GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat matSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat matShininess[] = { 30.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

        // 혹시 GL_COLOR_MATERIAL이 텍스처를 방해하면 끄는게 안전
        glDisable(GL_COLOR_MATERIAL);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_COLOR_MATERIAL);
    }

    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); i++) {
        Vertex& v = vertices[indices[i]];
        glNormal3f(v.normal.x, v.normal.y, v.normal.z);

        if (useTex) {
            glTexCoord2f(v.texCoord.x, v.texCoord.y);
        }

        glVertex3f(v.position.x, v.position.y, v.position.z);
    }
    glEnd();

    if (useTex) {
        glEnable(GL_COLOR_MATERIAL); // 다른 오브젝트 영향 없게 복구
        glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
}

// ========== PinManager  ==========
PinManager::PinManager() {
    ResetAll();
}

void PinManager::ResetAll() {
    for (int i = 0; i < 10; i++) {
        vec3 pos = PIN_POSITIONS[i];
        pos.z += PIN_START_Z;
        pins[i] = Pin(i + 1, pos);
    }
    standingCount = 10;
}

void PinManager::RemoveFallenPins() {
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

void PinManager::CheckBallCollision(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(nullptr));
        seeded = true;
    }

    // ✅ 포켓 스트라이크 확률 더 감소 (10% -> 5%)
    Pin& head = pins[0];
    bool headUp = pins[0].isStanding;
    bool pin2Up = pins[1].isStanding;
    bool pin3Up = pins[2].isStanding;

    if (headUp && (pin2Up || pin3Up)) {
        float dz = fabsf(ballPos.z - head.position.z);
        float zGate = (ballRadius + PIN_RADIUS) * 2.0f;
        float pocketHalf = PIN_SPACING * 0.3f;
        bool inPocketZone = (fabsf(ballPos.x) <= pocketHalf);

        if (dz < zGate && inPocketZone) {
            if ((rand() % 100) < 5) {  // ✅ 10% -> 5%
                float base = length(ballVelocity);
                float force = std::max(8.0f, base * 4.5f);

                for (int i = 0; i < 10; i++) {
                    if (!pins[i].isStanding) continue;

                    pins[i].isStanding = false;
                    pins[i].isFalling = true;

                    vec3 dir = pins[i].position - ballPos;
                    dir.y = 0.0f;
                    if (length(dir) < 0.001f) {
                        dir = vec3((rand() % 200 - 100) / 100.0f, 0.0f, -1.0f);
                    }
                    dir = normalize(dir);

                    float jitter = 0.8f + (rand() % 25) / 100.0f;
                    pins[i].velocity = dir * force * jitter * 0.5f;
                    pins[i].velocity.y = force * 0.12f;

                    pins[i].angularVelocity = vec3(
                        dir.z * force * 0.8f,
                        (float)(rand() % 40 - 20) / 20.0f,
                        -dir.x * force * 0.8f
                    );
                }

                // ✅ 포켓 스트라이크도 볼 속도 감소
                ballVelocity *= 0.4f;
                return;
            }
        }
    }

    // ✅ 일반 충돌 (ballVelocity를 참조로 전달하여 수정 가능)
    for (int i = 0; i < 10; i++) {
        pins[i].CheckCollisionWithBall(ballPos, ballRadius, ballVelocity, ballAngularVelocity);
    }
}

void PinManager::CheckPinCollisions() {
    // ✅ 충돌 반복 횟수 증가?? (정확도 향상)
    for (int iter = 0; iter < 4; iter++) {
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
        if (length(pins[i].velocity) > 0.08f || length(pins[i].angularVelocity) > 0.1f) {
            return false;
        }
    }
    return true;
}