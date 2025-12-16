#pragma once
#include "Common.h"
#include "Texture.h"

class Pin {
public:
    // ========== 물리 속성 ==========
    vec3 position;
    vec3 velocity;
    vec3 angularVelocity;
    float radius;
    float height;
    float mass;

    // ========== 회전 상태 ==========
    vec3 rotation;  // 오일러 각도 (x, y, z)

    // ========== 상태 플래그 ==========
    bool isStanding;    // 서있는지
    bool isFalling;     // 쓰러지는 중인지
    int pinNumber;      // 핀 번호 (1-10)

    // ========== ✅ 2번 코드:  물리 시뮬레이션 개선 ==========
    vec3 centerOfMass;  // 무게 중심
    float inertia;      // 관성 모멘트

    // ========== ✅ 1번 코드: 게임 상태 관리 ==========
    bool inPlay;        // 핀이 레인에 활성화된 상태인지 (쓰러지면 false)

    // ========== ✅ 1번 코드: 충돌 이펙트 (현재 미사용, 향후 확장용) ==========
    bool effectActive;      // 이펙트 활성화 여부
    float effectTimer;      // 이펙트 남은 시간 (초)
    static constexpr float EFFECT_DURATION = 0.3f;  // 이펙트 지속 시간

    // ========== 생성자 ==========
    Pin();
    Pin(int number, vec3 pos);

    // ========== 초기화 ==========
    void Reset();

    // ========== 물리 업데이트 ==========
    void Update(float dt);

    // ========== 충돌 처리 ==========
    // ✅ 2번 코드: ballVelocity를 참조로 받아 공도 영향 받음
    bool CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity);
    bool CheckCollisionWithPin(Pin& other);
    void ApplyImpact(vec3 impactDir, float force);

    // ========== 렌더링 ==========
    void Draw();

    // ========== 바닥 충돌 ==========
    void CheckFloor();

    // ========== 상태 확인 ==========
    bool IsDown();  // 완전히 쓰러졌는지

    // ========== ✅ 텍스처 로딩 (1번 + 2번 통합) ==========
    // 1번:  static void LoadTexture();
    // 2번: static void InitTexture(const char* filepath);
    // → 2번 방식 채택 (파일 경로 지정 가능)
    static void InitTexture(const char* filepath);

    // ✅ 1번 코드 호환성 유지 (내부적으로 InitTexture 호출)
    static void LoadTexture() {
        InitTexture("textures/pin. jpg");
    }

private:
    // ========== ✅ 텍스처 (1번 + 2번 통합) ==========
    // 1번: static GLuint texture;
    // 2번: static GLuint pinTextureID; static bool textureLoaded;
    // → 둘 다 유지 (호환성)
    static GLuint pinTextureID;
    static bool textureLoaded;

public:
    // ✅ 1번 코드 호환성:  texture는 pinTextureID의 별칭
    static GLuint& texture;  // 참조로 선언하여 pinTextureID와 동일하게 사용

private:
    // ========== ✅ 2번 코드: 커스텀 메쉬 ==========
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

// ========== PinManager (10개 핀 관리) ==========
class PinManager {
public:
    Pin pins[10];
    int standingCount;

    PinManager();

    // ========== 초기화 ==========
    void ResetAll();            // 모든 핀 초기화
    void RemoveFallenPins();    // 2투구용:  쓰러진 핀만 제거

    // ========== 업데이트 ==========
    void Update(float dt);

    // ========== 충돌 체크 ==========
    // ✅ 2번 코드: ballVelocity를 참조로 받음
    void CheckBallCollision(vec3 ballPos, float ballRadius, vec3& ballVelocity, vec3 ballAngularVelocity);
    void CheckPinCollisions();  // 핀끼리 충돌

    // ========== 렌더링 ==========
    void Draw();

    // ========== 상태 확인 ==========
    int CountStanding();    // 서있는 핀 수
    bool AllSettled();      // 모든 핀이 멈췄는지
};