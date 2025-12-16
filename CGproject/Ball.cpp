#include "Ball.h"
#include <cmath>
#include <algorithm>
#include "Texture.h"
#include "common.h"

static float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// 정적 멤버 초기화
GLuint Ball::textures[3] = { 0, 0, 0 };
GLuint Ball::currentTexture = 0;

void Ball::LoadTextures() {
    textures[0] = Texture::Load("textures/ball. jpg");
    textures[1] = Texture::Load("textures/ball1.jpg");
    textures[2] = Texture::Load("textures/ball2.jpg");
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

    float baseSpeed = 7.5f + shaped * 11.0f;

    if (spin == SpinType::STRAIGHT) {
        baseSpeed *= 1.25f;
    }
    else {
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
    pathStartX = position.x;
    pathStartZ = position.z;

    pathEndX = pathStartX;
    pathEndZ = PIN_START_Z;

    float safeHalf = (LANE_WIDTH * 0.5f) - BALL_RADIUS - 0.02f;
    if (safeHalf < 0.05f) safeHalf = 0.05f;

    float amp = safeHalf * 0.85f;
    float sign = (spin == SpinType::LEFT_HOOK) ? +1.0f : -1.0f;
    pathAmp = sign * amp;

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

    if (totalTime < 0.60f) totalTime = 0.60f;

    angularVelocity = vec3(0.0f, (spin == SpinType::LEFT_HOOK) ? 1.0f : -1.0f, 0.0f);
}

vec3 Ball::EvaluateCardinalSpline(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) {
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
            t = 1.0f;
            position = EvaluateSpline(t);
            useSpline = false;

            float keep = (splineSpeed > 0.1f) ? splineSpeed : 10.0f;
            velocity = vec3(0.0f, 0.0f, -keep);
        }
        else {
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

    if (spinType == SpinType::STRAIGHT) {
        frictionCoeff *= 0.45f;
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
    glDisable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    GLfloat matColor[4];
    GLfloat matAmbient[4];
    if (currentTexture != 0) {
        matColor[0] = matColor[1] = matColor[2] = 1.0f;
        matColor[3] = 1.0f;
        matAmbient[0] = matAmbient[1] = matAmbient[2] = 0.5f;
        matAmbient[3] = 1.0f;
    }
    else {
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

    glEnable(GL_TEXTURE_2D);
    if (currentTexture != 0) {
        Texture::Bind(currentTexture, 0);
    }
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

// [2번 추가] 그림자 그리기 함수
void Ball::DrawShadow(vec3 lightPos) {
    // 1. 조명과 텍스처 끄기 (그림자는 단색이어야 함)
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();

    // 2. 그림자 투영 행렬 생성
    GLfloat shadowMat[16];

    // 바닥 평면:  y = 0.001
    glm::vec4 groundPlane(0.0f, 1.0f, 0.0f, -0.001f);
    glm::vec4 lightPos4(lightPos.x, lightPos.y, lightPos.z, 1.0f);

    // Common. h에 추가된 함수 호출
    MakeShadowMatrix(shadowMat, lightPos4, groundPlane);

    // 그림자 투영 모델뷰 행렬에 곱하기
    glMultMatrixf(shadowMat);

    // 3. 실제 공의 위치로 그림자 (변환은 이미 적용됨)
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    // 4. 짙은 회색으로 렌더링
    glColor3f(0.15f, 0.15f, 0.15f);

    // 공 모양 그리기
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, radius, 32, 32);
    gluDeleteQuadric(quad);

    glPopMatrix();

    // 5. 복원 원복
    glEnable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f); // 기본 색상 복원
}

void Ball::SetBallType(int type) {
    if (type < 0) type = 0;
    if (type > 2) type = 2;
    ballType = type;
    currentTexture = textures[type];
}