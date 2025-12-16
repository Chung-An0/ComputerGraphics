#pragma once
#include "Common.h"

class Ball {
public:
    vec3 position;
    vec3 velocity;
    vec3 angularVelocity;

    float radius;
    float mass;

    float rotationAngle;
    vec3 rotationAxis;

    SpinType spinType;
    bool isRolling;
    bool isInGutter;
    int  ballType;

    // Hook Path (안정형 S-curve)
    float rollTime;
    float totalTime;
    bool  useSpline;
    float splineSpeed;

    float pathStartX, pathEndX;
    float pathStartZ, pathEndZ;
    float pathAmp;

    Ball();

    void Reset();
    void Reset(float startX);

    void Launch(float power, SpinType spin);
    void Update(float dt);

    void ApplyFriction(float dt);
    void CheckGutter();

    // (호환용)
    vec3 EvaluateCardinalSpline(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t);

    // 고정 경로
    void SetupSpline(float baseSpeed, SpinType spin);
    vec3 EvaluateSpline(float t); // 0..1

    void Draw();
    void SetBallType(int type);

    bool IsStopped();

    // --- 텍스처 관련 ---
    // 여러 색상의 공을 위해 3개의 텍스처를 로드한다. 메뉴에서 선택된 타입에 따라 다른 텍스처를 사용한다.
    // textures[0] → ball.jpg, textures[1] → ball1.jpg, textures[2] → ball2.jpg
    static GLuint textures[3];
    // 현재 렌더링에 사용할 텍스처 ID. SetBallType()에서 업데이트된다.
    static GLuint currentTexture;
    // 텍스처 로딩 함수: Game::Init()에서 호출하여 textures/ball.jpg, ball1.jpg, ball2.jpg를 읽는다.
    static void LoadTextures();
};
