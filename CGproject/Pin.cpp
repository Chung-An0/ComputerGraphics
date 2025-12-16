#include "Pin.h"
#include "Camera.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Texture.h"

// Windows 헤더와 사운드 재생을 위한 mmsystem을 포함한다.
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

// 핀 배치 위치 (삼각형)
//        7  8  9  10
//          4  5  6
//            2  3
//              1
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

// 정적 멤버 초기화
GLuint Pin::texture = 0;

// 텍스처 로딩
void Pin::LoadTexture() {
    // textures 폴더에 pin.jpg 파일이 있어야 합니다.
    texture = Texture::Load("textures/pin.jpg");
}

Pin::Pin() {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;
    mass = 1.5f;
    pinNumber = 0;
    Reset();

    // 이펙트 초기 상태
    effectActive = false;
    effectTimer = 0.0f;

    // 핀은 처음에 게임에 포함된다
    inPlay = true;
}

Pin::Pin(int number, vec3 pos) {
    radius = PIN_RADIUS;
    height = PIN_HEIGHT;
    mass = 1.5f;
    pinNumber = number;
    position = pos;
    position.y = height / 2.0f;
    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);

    // 이펙트 초기화
    effectActive = false;
    effectTimer = 0.0f;

    // 새 핀은 레인에 존재한다
    inPlay = true;
}

void Pin::Reset() {
    isStanding = true;
    isFalling = false;
    velocity = vec3(0.0f);
    angularVelocity = vec3(0.0f);
    rotation = vec3(0.0f);
    position.y = height / 2.0f;

    // 이펙트 중지
    effectActive = false;
    effectTimer = 0.0f;

    // 핀을 다시 활성화
    inPlay = true;
}

void Pin::Update(float dt) {
    // 게임에서 제거된 핀은 업데이트하지 않는다
    if (!inPlay) return;

    // 이펙트 갱신은 사용하지 않음

    // 쓰러진 핀은 바로 레인에서 제거한다
    if (!isStanding && isFalling) {
        inPlay = false;
        return;
    }

    if (isStanding && !isFalling) return;

    velocity.y += GRAVITY * dt;
    position += velocity * dt;

    rotation += angularVelocity * dt * 50.0f;

    velocity *= 0.98f;
    angularVelocity *= 0.95f;

    CheckFloor();

    if (abs(rotation.x) > 45.0f || abs(rotation.z) > 45.0f) {
        isStanding = false;
    }

    // 레인 밖으로 크게 벗어나거나 너무 높이 날아간 핀은 즉시 제거하여 화면에 남지 않도록 한다
    {
        // 레인의 중앙에서 측면 벽까지의 거리(여유를 포함)
        float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 0.5f;

        // x 축이 벽을 넘어가거나, z가 레인 범위를 벗어나거나, y가 너무 높으면 제거한다.  
        // "standing" 여부와 무관하게, 벽에 닿은 핀은 모두 제거하여 벽에 붙어서 남는 현상을 방지한다.
        if (fabs(position.x) > wallDist + 0.2f || position.z > 4.0f || position.z < -LANE_LENGTH - 1.0f || position.y > 1.0f) {
            inPlay = false;
        }
        // 측면 벽 근처까지 이동한 핀은 즉시 게임에서 제외한다.
        else if (fabs(position.x) >= (wallDist - radius)) {
            inPlay = false;
        }
    }
}

bool Pin::CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3 ballVelocity, vec3 ballAngularVelocity) {
    // 게임에서 제거된 핀은 충돌하지 않는다
    if (!inPlay) return false;
    if (!isStanding) return false;

    vec3 toPin = position - ballPos;
    toPin.y = 0;

    float dist = length(toPin);

    // 공-핀 충돌 판정(네가 이미 조절한 값 유지 가능)
    float collisionDist = (radius + ballRadius) * 1.3f;
    // collisionDist *= 1.40f; // 필요하면 여기서 40% 확대 (이건 "충돌판정"이고 스트라이크 확률이 아님)

    if (dist < collisionDist) {
        isFalling = true;

        vec3 impactDir = normalize(toPin);
        float impactForce = length(ballVelocity) * 3.0f;

        velocity = impactDir * impactForce * 0.8f;

        vec3 spinEffect = cross(ballAngularVelocity, impactDir) * 0.4f;
        velocity += spinEffect;

        velocity.y = impactForce * 0.4f;

        angularVelocity.x = -impactDir.z * impactForce * 2.0f + ballAngularVelocity.y * 0.5f;
        angularVelocity.y = (float)(rand() % 60 - 30) / 10.0f;
        angularVelocity.z = impactDir.x * impactForce * 2.0f - ballAngularVelocity.y * 0.5f;

        // 충돌 시 사운드만 재생 (시각적 이펙트 비활성화)
#ifdef _WIN32
        PlaySound(TEXT("sounds\\pin_hit.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif

        return true;
    }

    return false;
}

bool Pin::CheckCollisionWithPin(Pin& other) {
    // 게임에서 제거된 핀과는 충돌하지 않는다
    if (!inPlay || !other.inPlay) return false;
    if (!other.isFalling && !other.isStanding) return false;
    if (this == &other) return false;

    vec3 diff = other.position - position;
    float dist = length(diff);
    float collisionDist = radius * 2.8f;

    if (dist < collisionDist && dist > 0.001f) {
        // 충돌 시 사운드만 재생 (이펙트 비활성화)
#ifdef _WIN32
        PlaySound(TEXT("sounds\\pin_hit.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif

        vec3 normal = normalize(diff);

        float overlap = collisionDist - dist;
        position -= normal * overlap * 0.5f;
        other.position += normal * overlap * 0.5f;

        vec3 relVel = velocity - other.velocity;
        float velAlongNormal = dot(relVel, normal);

        if (velAlongNormal > 0) return false;

        float restitution = 0.8f;
        float impulse = -(1 + restitution) * velAlongNormal / 2.0f;

        vec3 impulseVec = impulse * normal;
        velocity += impulseVec;
        other.velocity -= impulseVec;

        if (isFalling && other.isStanding) {
            float hitForce = length(velocity);
            if (hitForce > 0.2f) {
                other.isStanding = false;
                other.isFalling = true;
                other.velocity = normal * hitForce * 0.6f;
                other.velocity.y = 0.8f;

                other.angularVelocity = vec3(
                    normal.z * hitForce * 2.0f,
                    (float)(rand() % 40 - 20) / 10.0f,
                    -normal.x * hitForce * 2.0f
                );
            }
        }

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

        if (isFalling && other.isFalling) {
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

    velocity = impactDir * force * 0.8f;
    velocity.y = force * 0.3f;

    angularVelocity.x = -impactDir.z * force * 2.0f;
    angularVelocity.z = impactDir.x * force * 2.0f;
}

void Pin::CheckFloor() {
    if (position.y < height / 2.0f) {
        position.y = height / 2.0f;
        velocity.y = 0;

        velocity.x *= 0.9f;
        velocity.z *= 0.9f;
    }

    if (IsDown()) {
        velocity = vec3(0.0f);
        angularVelocity = vec3(0.0f);
    }
}

bool Pin::IsDown() {
    return !isStanding || abs(rotation.x) > 80.0f || abs(rotation.z) > 80.0f;
}

void Pin::Draw() {
    // 게임에서 제거된 핀은 그리지 않는다
    if (!inPlay) return;

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);

    // 스펙큘러와 광택 설정: 텍스처와 함께 적용
    GLfloat matSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat matShininess[] = { 50.0f };
    // 확산색을 흰색으로 지정하여 텍스처가 본래 색상대로 표현되도록 한다
    GLfloat matDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // 텍스처 활성화 및 바인딩
    glEnable(GL_TEXTURE_2D);
    if (texture != 0) {
        Texture::Bind(texture, 0);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);

    // 핀 몸통 (아래 굵은 부분 -> 중간 -> 좁아지는 부분)
    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f + 0.02f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius * 1.2f, radius, height * 0.3f, 16, 4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f + height * 0.3f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius, radius * 0.6f, height * 0.4f, 16, 4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f + height * 0.65f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, radius * 0.6f, radius * 0.5f, height * 0.1f, 16, 2);
    glPopMatrix();

    // 핀 상단 구체
    glPushMatrix();
    glTranslatef(0.0f, height / 2.0f - radius * 0.8f, 0.0f);
    gluSphere(quad, radius * 0.7f, 16, 16);
    glPopMatrix();

    gluDeleteQuadric(quad);
    if (texture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

// ============ PinManager ============

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
        // 쓰러진 핀은 레인에서 제거한다
        if (!pins[i].isStanding || pins[i].IsDown()) {
            pins[i].inPlay = false;
        }
        else {
            // 아직 서있는 핀은 속도를 0으로 고정하여 흔들리지 않게 한다
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
    // 랜덤 시드(한 번만)
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(nullptr));
        seeded = true;
    }

    // ====== ✅ "40% 확률 강제 스트라이크" 조건 ======
    // 공이 1번 핀 근처에 들어오고, x가 "1-2와 1-3 사이" (중앙 포켓 존) 안에 있으면
    // 40% 확률로 모든 핀을 즉시 쓰러뜨린다.
    Pin& head = pins[0]; // 1번 핀

    bool headUp = pins[0].isStanding; // 1번(헤드핀)
    bool pin2Up = pins[1].isStanding; // 2번
    bool pin3Up = pins[2].isStanding; // 3번

    // 1번이 서 있고, 2번 또는 3번 중 하나라도 서 있을 때만 "포켓 스트라이크" 발동
    if (headUp && (pin2Up || pin3Up)) {

        float dz = fabsf(ballPos.z - head.position.z);

        // 공이 헤드핀 근처까지 왔는지(너무 멀면 판정 X)
        float zGate = (ballRadius + PIN_RADIUS) * 2.2f;

        // "1-2와 1-3 사이"를 x 범위로 정의 (너무 좁지 않게)
        // - PIN_SPACING 기반으로 잡으면 핀 배치랑 자연스럽게 맞는다.
        float pocketHalf = PIN_SPACING * 0.35f; // 범위 조절(더 자주면 0.45f)
        bool inPocketZone = (fabsf(ballPos.x) <= pocketHalf);

        if (dz < zGate && inPocketZone) {
            if ((rand() % 100) < 20) { // ✅ 20% 확률
                float base = length(ballVelocity);
                float force = std::max(10.0f, base * 6.0f);

                for (int i = 0; i < 10; i++) {
                    if (!pins[i].isStanding) continue;

                    // 즉시 "쓰러진 상태"로 전환
                    pins[i].isStanding = false;
                    pins[i].isFalling = true;

                    // 바깥으로 튀게
                    vec3 dir = pins[i].position - ballPos;
                    dir.y = 0.0f;
                    if (length(dir) < 0.001f) {
                        dir = vec3((rand() % 200 - 100) / 100.0f, 0.0f, -1.0f);
                    }
                    dir = normalize(dir);

                    float jitter = 0.85f + (rand() % 30) / 100.0f; // 0.85 ~ 1.14
                    pins[i].velocity = dir * force * jitter;
                    pins[i].velocity.y = force * 0.55f;

                    pins[i].angularVelocity = vec3(
                        dir.z * force * 1.5f,
                        (float)(rand() % 80 - 40) / 10.0f,
                        -dir.x * force * 1.5f
                    );
                }

                return; // 강제 스트라이크 처리했으면 일반 충돌은 생략
            }
        }
    }

    // ====== 일반 충돌 처리 ======
    for (int i = 0; i < 10; i++) {
        if (!pins[i].inPlay) continue;
        pins[i].CheckCollisionWithBall(ballPos, ballRadius, ballVelocity, ballAngularVelocity);
    }
}

void PinManager::CheckPinCollisions() {
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

// 카메라 정보를 받아, 카메라 앞쪽에 있는 핀만 그리는 오버로드된 드로우 함수입니다.
// 단순히 카메라의 전방 벡터와 핀까지의 벡터의 내적을 이용하여, 음수일 경우(카메라 뒤쪽) 렌더링을 생략합니다.
void PinManager::Draw(const Camera& cam) {
    // 카메라 위치와 전방 방향을 구한다.
    vec3 camPos = cam.position;
    vec3 camForward = cam.GetForward();
    for (int i = 0; i < 10; i++) {
        // 레인에 남아 있는 핀만 고려한다.
        if (!pins[i].inPlay) continue;
        // 카메라에서 핀까지의 벡터
        vec3 toPin = pins[i].position - camPos;
        // 내적이 양수이면 카메라 앞쪽에 있다. 0이면 정면, 음수이면 뒤쪽.
        float dotVal = dot(camForward, normalize(toPin));
        if (dotVal > 0.0f) {
            pins[i].Draw();
        }
    }
}

int PinManager::CountStanding() {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (!pins[i].inPlay) continue;
        if (pins[i].isStanding && !pins[i].IsDown()) {
            count++;
        }
    }
    return count;
}

bool PinManager::AllSettled() {
    for (int i = 0; i < 10; i++) {
        if (!pins[i].inPlay) continue;
        if (length(pins[i].velocity) > 0.05f) {
            return false;
        }
    }
    return true;
}
