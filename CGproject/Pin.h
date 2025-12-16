#pragma once
#include "Common.h"
#include "Texture.h"

class Pin {
public:
    vec3 position;
    vec3 velocity;
    vec3 angularVelocity;
    float radius;
    float height;
    float mass;

    vec3 rotation;

    bool isStanding;
    bool isFalling;
    int pinNumber;

    // ✅ 추가:  물리 시뮬레이션 개선
    vec3 centerOfMass;    // 무게 중심
    float inertia;        // 관성 모멘트

    Pin();
    Pin(int number, vec3 pos);

    void Reset();
    void Update(float dt);

    // ✅ ballVelocity를 참조로 변경 (공도 영향 받음)
    bool CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity);
    bool CheckCollisionWithPin(Pin& other);
    void ApplyImpact(vec3 impactDir, float force);

    void Draw();
    void CheckFloor();
    bool IsDown();

    // 텍스처 초기화 (Texture::Load로)
    static void InitTexture(const char* filepath);

private:
    static GLuint pinTextureID;
    static bool textureLoaded;

    struct Vertex {
        vec3 position;
        vec3 normal;
        vec2 texCoord;
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    bool meshGenerated;

    void GenerateRevolutionMesh();
    float EvaluateCardinalSpline1D(float p0, float p1, float p2, float p3, float t);
    vec2 GetProfilePoint(float t);
};

// PinManager는 그대로
class PinManager {
public:
    Pin pins[10];
    int standingCount;

    PinManager();
    void ResetAll();
    void RemoveFallenPins();
    void Update(float dt);
    // ✅ ballVelocity를 참조로 변경
    void CheckBallCollision(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity);
    void CheckPinCollisions();
    void Draw();
    int CountStanding();
    bool AllSettled();
};