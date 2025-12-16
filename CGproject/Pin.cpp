#include "Pin.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

// static 멤버 변수 초기화
GLuint Pin::pinTextureID = 0;
bool Pin::textureLoaded = false;

//텍스처는 핀텍스처id 참조
GLuint& Pin::texture = Pin::pinTextureID;

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

// ========== 볼링핀 프로필 곡선 ==========
vec2 Pin::GetProfilePoint(float t) {
    const int numControlPoints = 8;
    float controlY[8] = {
        0.00f, 0.25f, 0.70f, 0.85f,
        0.90f, 0.95f, 0.98f, 1.00f
    };

    float controlRadius[8] = {
        0.028f, 0.056f, 0.020f, 0.023f,
        0.018f, 0.012f, 0.002f, 0.001f
    };

    int segment = 0;
    for (int i = 0; i < numControlPoints - 1; i++) {
        if (t >= controlY[i] && t <= controlY[i + 1]) {
            segment = i;
            break;
        }
    }

    if (segment == 0 && t < controlY[0]) segment = 0;
    if (segment >= numControlPoints - 2) segment = numControlPoints - 3;

    int i0 = std::max(0, segment - 1);
    int i1 = segment;
    int i2 = std::min(numControlPoints - 1, segment + 1);
    int i3 = std::min(numControlPoints - 1, segment + 2);

    float localT = 0.0f;
    if (controlY[i2] - controlY[i1] > 0.0001f) {
        localT = (t - controlY[i1]) / (controlY[i2] - controlY[i1]);
    }
    localT = std::max(0.0f, std::min(1.0f, localT));

    float radius = EvaluateCardinalSpline1D(
        controlRadius[i0],
        controlRadius[i1],
        controlRadius[i2],
        controlRadius[i3],
        localT
    );

    if (radius < 0.001f) radius = 0.001f;
    float y = t * height;

    return vec2(radius, y);
}

// ========== 회전체 메쉬 생성 ==========
void Pin::GenerateRevolutionMesh() {
    if (meshGenerated) return;

    vertices.clear();
    indices.clear();

    const int heightSegments = 30;
    const int radialSegments = 32;

    for (int i = 0; i <= heightSegments; i++) {
        float t = (float)i / (float)heightSegments;
        t = t * 0.90f;

        vec2 profile = GetProfilePoint(t);
        float r = profile.x;
        float y = profile.y - height / 2.0f;

        float eps = 0.01f;
        vec2 profileNext = GetProfilePoint(std::min(1.0f, t + eps));
        vec2 tangent2D = normalize(profileNext - profile);

        for (int j = 0; j <= radialSegments; j++) {
            float angle = (float)j / (float)radialSegments * 2.0f * 3.14159265f;

            Vertex v;
            v.position = vec3(r * cos(angle), y, r * sin(angle));

            vec3 radialDir = normalize(vec3(cos(angle), 0.0f, sin(angle)));
            vec3 heightDir = vec3(0.0f, tangent2D.y, 0.0f);
            v.normal = normalize(radialDir - heightDir * tangent2D.x);

            v.texCoord = vec2(
                (float)j / (float)radialSegments,
                1.0f - (t / 0.90f)
            );

            vertices.push_back(v);
        }
    }

    for (int i = 0; i < heightSegments; i++) {
        for (int j = 0; j < radialSegments; j++) {
            int current = i * (radialSegments + 1) + j;
            int next = current + radialSegments + 1;

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
    mass = 1.5f;
    pinNumber = 0;
    meshGenerated = false;

    centerOfMass = vec3(0.0f, height * 0.35f, 0.0f);
    inertia = mass * radius * radius * 0.4f;

    // ✅ 합친 코드 기능: inPlay
    inPlay = true;
    effectActive = false;
    effectTimer = 0.0f;

    Reset();
    GenerateRevolutionMesh();
}

Pin::Pin(int number, vec3 pos) {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;

    // ✅ 원래 “자연스럽던” 물리 파라미터 그대로 유지 (중요)
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

    // ✅ 합친 코드 기능: inPlay
    inPlay = true;
    effectActive = false;
    effectTimer = 0.0f;

    GenerateRevolutionMesh();
}

void Pin::Reset() {
    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);
    position.y = height / 2.0f;

    // ✅ 합친 코드 기능
    inPlay = true;
    effectActive = false;
    effectTimer = 0.0f;
}

// ========== 물리 업데이트 ==========
void Pin::Update(float dt) {
    if (!inPlay) return;
    if (isStanding && !isFalling) return;

    velocity.y += GRAVITY * dt;
    position += velocity * dt;

    rotation += angularVelocity * dt * 57.2958f;

    // ✅ 회전 각도 제한 (360도 넘어가지 않도록)
    if (rotation.x > 180.0f) rotation.x -= 360.0f;
    if (rotation.x < -180.0f) rotation.x += 360.0f;
    if (rotation.z > 180.0f) rotation.z -= 360.0f;
    if (rotation.z < -180.0f) rotation.z += 360.0f;

    velocity *= 0.97f;
    angularVelocity *= 0.88f;

    CheckFloor();

    if (abs(rotation.x) > 35.0f || abs(rotation.z) > 35.0f) {
        isStanding = false;
    }

    // ✅ 쓰러진 핀의 안정화 개선
    if (IsDown()) {
        // 바닥에 닿았을 때 회전을 빠르게 감쇠
        if (position.y <= height / 2.0f * 0.5f) {  // 높이가 절반 이하
            angularVelocity *= 0.92f;  // 회전 더 빨리 멈춤
        }

        if (length(velocity) < 0.1f) {
            velocity *= 0.9f;
            angularVelocity *= 0.85f;

            if (length(velocity) < 0.05f && length(angularVelocity) < 0.05f) {
                velocity = vec3(0.0f);
                angularVelocity = vec3(0.0f);
            }
        }
    }

    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 0.5f;
    if (fabs(position.x) > wallDist + 0.2f ||
        position.z > 4.0f ||
        position.z < -LANE_LENGTH - 1.0f ||
        position.y > 1.0f) {
        inPlay = false;
        return;
    }
    else if (fabs(position.x) >= (wallDist - radius)) {
        inPlay = false;
        return;
    }
}
// ========== 볼-핀 충돌 (수정)
bool Pin::CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity) {
    if (!inPlay) return false;

    vec3 toPin = position - ballPos;
    float dist = length(toPin);
    float collisionDist = (radius + ballRadius) * 1.15f;

    if (dist < collisionDist && dist > 0.001f) {
        vec3 impactDir = normalize(toPin);
        float ballSpeed = length(ballVelocity);

        // 볼링공이 훨씬 무겁게 (6. 8kg → 7.5kg)
        float ballMass = 7.5f;
        float totalMass = ballMass + mass;

        if (isStanding) {
            isFalling = true;
            isStanding = false;

            float impactHeight = ballPos.y - (position.y - height / 2.0f);
            impactHeight = std::max(0.0f, std::min(height, impactHeight));
            float heightRatio = impactHeight / height;

            // 핀이 더 약하게 반응 (반발계수 낮춤)
            float restitution = 0.25f;  // 0.4f → 0.25f
            vec3 ballVelNormal = dot(ballVelocity, impactDir) * impactDir;
            float ballVelMag = length(ballVelNormal);

            // 핀이 받는 속도 계산 (공의 질량이 훨씬 크므로 핀이 많이 튕김)
            float pinSpeed = (2.0f * ballMass * ballVelMag) / totalMass;

            // 핀의 속도 - 수평은 조금 감소, 수직은 더 감소
            velocity = impactDir * pinSpeed * 0.50f;  // 0.55f → 0.50f
            velocity.y = pinSpeed * 0.10f * (0.8f + heightRatio * 0.3f);  // 0.12f → 0.10f

            // 스핀 효과도 약하게
            vec3 spinEffect = cross(ballAngularVelocity, impactDir) * 0.10f;  // 0.15f → 0.10f
            velocity += spinEffect;

            // 토크 계산 (회전 약하게)
            vec3 leverArm = vec3(0.0f, impactHeight - centerOfMass.y, 0.0f);
            vec3 impactForce = impactDir * pinSpeed;
            vec3 torque = cross(leverArm, impactForce);

            angularVelocity = torque / inertia * 0.7f;  // 0.8f → 0.7f
            angularVelocity.x += -impactDir.z * pinSpeed * 0.85f;  // 0.9f → 0.85f
            angularVelocity.z += impactDir.x * pinSpeed * 0.85f;
            angularVelocity.y = (float)(rand() % 20 - 10) / 35.0f;  // 30. 0f → 35.0f

            // 볼링공은 거의 영향 안 받음 (질량이 5배 차이)
            float ballSpeedLoss = (mass * pinSpeed) / ballMass * 0.8f;  // 계수 추가
            ballVelocity -= impactDir * ballSpeedLoss * (1.0f + restitution) * 0.6f;  // 0.6배 감소

            // 볼링공 최소 속도 보장 (공이 거의 안 느려짐)
            if (length(ballVelocity) < ballSpeed * 0.4f) {  // 0.3f → 0.4f
                ballVelocity = normalize(ballVelocity) * ballSpeed * 0.4f;
            }
        }
        else {
            //  쓰러진 핀 - 공은 거의 영향 없음
            float restitution = 0.10f;  // 0.15f → 0.10f (더 약하게)

            vec3 relativeVel = ballVelocity - velocity;
            float velAlongNormal = dot(relativeVel, impactDir);

            if (velAlongNormal < 0) return false;

            float massRatio = mass / ballMass;  // 약 0.20 (1: 5 비율)
            float impulse = (1.0f + restitution) * velAlongNormal / (1.0f + massRatio);

            vec3 pushDir = impactDir;
            pushDir.y = 0;
            if (length(pushDir) > 0.001f) {
                pushDir = normalize(pushDir);

                //  쓰러진 핀도 약하게만 밀림
                velocity.x += pushDir.x * impulse * massRatio * 0.35f;  // 0.4f → 0.35f
                velocity.z += pushDir.z * impulse * massRatio * 0.35f;
                velocity.y += impulse * massRatio * 0.015f;  // 0.02f → 0.015f

                //  회전도 약하게
                angularVelocity += vec3(
                    pushDir.z * impulse * massRatio * 0.15f,  // 0.2f → 0.15f
                    0.0f,
                    -pushDir.x * impulse * massRatio * 0.15f
                );
            }

            //  볼링공은 거의 영향 없음
            ballVelocity -= impactDir * impulse * 0.05f;  // 0.08f → 0.05f
        }

#ifdef _WIN32
        PlaySound(TEXT("sounds\\pin_hit.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif
        return true;
    }

    return false;
}

// ========== 핀-핀 충돌 ==========
bool Pin::CheckCollisionWithPin(Pin& other) {
    //  합친 코드 기능: 제거된 핀과는 충돌 안 함
    if (!inPlay || !other.inPlay) return false;

    if (!isFalling && !isStanding && !other.isFalling && !other.isStanding) {
        if (length(velocity) < 0.01f && length(other.velocity) < 0.01f) {
            return false;
        }
    }

    if (this == &other) return false;

    vec3 diff = other.position - position;
    diff.y = 0;
    float dist = length(diff);
    float collisionDist = radius * 2.3f;

    if (dist < collisionDist && dist > 0.001f) {
        vec3 normal = normalize(diff);

        float overlap = collisionDist - dist;
        position -= normal * overlap * 0.5f;
        other.position += normal * overlap * 0.5f;

        vec3 relVel = velocity - other.velocity;
        relVel.y = 0;
        float velAlongNormal = dot(relVel, normal);

        if (velAlongNormal > 0) return false;

        float restitution = 0.35f;
        float impulse = -(1 + restitution) * velAlongNormal / 2.0f;

        vec3 impulseVec = impulse * normal;
        velocity.x += impulseVec.x;
        velocity.z += impulseVec.z;
        other.velocity.x -= impulseVec.x;
        other.velocity.z -= impulseVec.z;

        if (isFalling && other.isStanding) {
            float hitForce = length(vec3(velocity.x, 0, velocity.z));
            if (hitForce > 0.25f) {
                other.isStanding = false;
                other.isFalling = true;

                float impactHeight = position.y - (other.position.y - other.height / 2.0f);
                impactHeight = std::max(0.0f, std::min(other.height * 0.6f, impactHeight));
                float heightRatio = impactHeight / other.height;

                other.velocity = normal * hitForce * 0.5f;
                other.velocity.y = hitForce * 0.08f * (1.0f + heightRatio * 0.3f);

                vec3 leverArm = vec3(0.0f, impactHeight - other.centerOfMass.y, 0.0f);
                vec3 force = normal * hitForce;
                vec3 torque = cross(leverArm, force);

                other.angularVelocity = torque / other.inertia * 0.7f;
                other.angularVelocity.x += normal.z * hitForce * 0.8f;
                other.angularVelocity.z += -normal.x * hitForce * 0.8f;
                other.angularVelocity.y = (float)(rand() % 20 - 10) / 25.0f;
            }
        }

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

        if (isFalling && other.isFalling) {
            velocity *= 0.95f;
            other.velocity *= 0.95f;

            vec3 avgAngVel = (angularVelocity + other.angularVelocity) * 0.5f;
            angularVelocity = avgAngVel * 0.85f;
            other.angularVelocity = avgAngVel * 0.85f;
        }

        // ✅ 합친 코드 기능: 핀-핀 충돌 사운드
#ifdef _WIN32
        PlaySound(TEXT("sounds\\pin_hit.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif
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

// ========== CheckFloor 함수 (완전히 개선) ==========
void Pin::CheckFloor() {
    // ✅ 회전 각도 계산
    float rotMag = sqrt(rotation.x * rotation.x + rotation.z * rotation.z);
    float tiltRatio = std::min(1.0f, rotMag / 90.0f);

    float minY;
    if (isStanding) {
        minY = height / 2.0f;
    }
    else {
        // ✅ 쓰러진 핀의 바닥 높이를 더 정확하게 계산
        // 완전히 쓰러지면 (90도) 핀이 옆으로 누워야 함
        // 이때 높이는 핀의 최대 반지름
        float maxRadius = 0.056f;  // GetProfilePoint에서 가장 큰 값

        // 각도에 따라 선형 보간
        // 0도 (서있음) → height/2
        // 90도 (완전히 쓰러짐) → maxRadius
        minY = (1.0f - tiltRatio) * (height / 2.0f) + tiltRatio * maxRadius;

        // ✅ 너무 낮아지지 않도록 최소값 보장
        if (minY < maxRadius * 0.8f) minY = maxRadius * 0.8f;
    }

    if (position.y < minY) {
        position.y = minY;

        // ✅ 바닥 반발 거의 없음
        if (velocity.y < -0.1f) {
            velocity.y = -velocity.y * 0.08f;  // 0.10f → 0.08f (더 약하게)
        }
        else {
            velocity.y = 0;
        }

        // ✅ 바닥 마찰 강화
        velocity.x *= 0.82f;  // 0.85f → 0.82f
        velocity.z *= 0.82f;
        angularVelocity *= 0.78f;  // 0.82f → 0.78f
    }

    // ✅ 쓰러진 후 안정화 (회전을 더 빨리 멈춤)
    if (IsDown()) {
        if (length(velocity) < 0.12f) {
            velocity *= 0.85f;  // 0.88f → 0.85f
            angularVelocity *= 0.75f;  // 0.80f → 0.75f

            if (length(velocity) < 0.05f) velocity = vec3(0.0f);
            if (length(angularVelocity) < 0.05f) angularVelocity = vec3(0.0f);
        }

        // ✅ 완전히 멈췄을 때 회전도 수평에 가깝게 보정
        if (length(velocity) < 0.02f && length(angularVelocity) < 0.02f) {
            // 회전을 90도에 가깝게 스냅 (옆으로 완전히 누운 상태)
            if (abs(rotation.x) > 80.0f) {
                rotation.x = (rotation.x > 0) ? 90.0f : -90.0f;
            }
            if (abs(rotation.z) > 80.0f) {
                rotation.z = (rotation.z > 0) ? 90.0f : -90.0f;
            }

            velocity = vec3(0.0f);
            angularVelocity = vec3(0.0f);
        }
    }
}

bool Pin::IsDown() {
    return !isStanding || abs(rotation.x) > 75.0f || abs(rotation.z) > 75.0f;
}

void Pin::Draw() {
    // ✅ 합친 코드 기능: 제거된 핀은 그리지 않음
    if (!inPlay) return;

    if (!meshGenerated) GenerateRevolutionMesh();

    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);

    const bool useTex = (textureLoaded && pinTextureID != 0);

    if (useTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, pinTextureID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat matSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat matShininess[] = { 30.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

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
        if (useTex) glTexCoord2f(v.texCoord.x, v.texCoord.y);
        glVertex3f(v.position.x, v.position.y, v.position.z);
    }
    glEnd();

    if (useTex) {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
}

// ========== PinManager ==========
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

// ✅ “턴 종료 정리” 함수로 동작하게 변경:
// - 애니메이션 도중에는 건드리지 말고
// - 프레임이 아니라 “턴 끝(공 멈추고 AllSettled=true)” 시점에 호출하는 걸 권장
void PinManager::RemoveFallenPins() {
    for (int i = 0; i < 10; i++) {
        if (!pins[i].inPlay) continue;

        // 3번 요구: 서있지 않으면(또는 IsDown이면) 쓰러진 걸로 보고 제거
        if (!pins[i].isStanding || pins[i].IsDown()) {
            pins[i].inPlay = false;
        }
        else {
            // 서있는 핀은 흔들림 제거
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

    // ✅ 합친 코드 기능: 포켓 스트라이크 확률 15%
    Pin& head = pins[0];
    bool headUp = pins[0].inPlay && pins[0].isStanding;
    bool pin2Up = pins[1].inPlay && pins[1].isStanding;
    bool pin3Up = pins[2].inPlay && pins[2].isStanding;

    if (headUp && (pin2Up || pin3Up)) {
        float dz = fabsf(ballPos.z - head.position.z);
        float zGate = (ballRadius + PIN_RADIUS) * 2.0f;
        float pocketHalf = PIN_SPACING * 0.3f;
        bool inPocketZone = (fabsf(ballPos.x) <= pocketHalf);

        if (dz < zGate && inPocketZone) {
            if ((rand() % 100) < 15) {
                float base = length(ballVelocity);
                float force = std::max(8.0f, base * 5.0f);

                for (int i = 0; i < 10; i++) {
                    if (!pins[i].inPlay) continue;
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

                ballVelocity *= 0.4f;
                return;
            }
        }
    }

    for (int i = 0; i < 10; i++) {
        if (!pins[i].inPlay) continue;
        pins[i].CheckCollisionWithBall(ballPos, ballRadius, ballVelocity, ballAngularVelocity);
    }
}

void PinManager::CheckPinCollisions() {
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
        if (!pins[i].inPlay) continue;
        if (pins[i].isStanding && !pins[i].IsDown()) count++;
    }
    return count;
}

bool PinManager::AllSettled() {
    for (int i = 0; i < 10; i++) {
        if (!pins[i].inPlay) continue;
        if (length(pins[i].velocity) > 0.08f || length(pins[i].angularVelocity) > 0.1f) {
            return false;
        }
    }
    return true;
}