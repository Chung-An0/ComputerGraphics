#pragma once
#include "Common.h"

class Pin {
public:
    // 물리 속성
    vec3 position;
    vec3 velocity;
    vec3 angularVelocity;
    float radius;
    float height;
    float mass;
    
    // 회전 상태
    vec3 rotation;      // 오일러 각도 (x, y, z)
    
    // 상태
    bool isStanding;    // 서있는지
    bool isFalling;     // 쓰러지는 중인지
    int pinNumber;      // 핀 번호 (1-10)
    
    // 생성자
    Pin();
    Pin(int number, vec3 pos);
    
    // 초기화
    void Reset();
    
    // 물리 업데이트
    void Update(float dt);
    
    // 공과 충돌 체크
    bool CheckCollisionWithBall(vec3 ballPos, float ballRadius, vec3 ballVelocity);
    
    // 다른 핀과 충돌 체크
    bool CheckCollisionWithPin(Pin& other);
    
    // 충돌 반응
    void ApplyImpact(vec3 impactDir, float force);
    
    // 렌더링
    void Draw();
    
    // 바닥 충돌 처리
    void CheckFloor();
    
    // 완전히 쓰러졌는지
    bool IsDown();
};

// 10개 핀 관리 클래스
class PinManager {
public:
    Pin pins[10];
    int standingCount;
    
    PinManager();
    
    // 모든 핀 초기화
    void ResetAll();
    
    // 2투구용: 쓰러진 핀만 제거
    void RemoveFallenPins();
    
    // 업데이트
    void Update(float dt);
    
    // 공과 충돌 체크
    void CheckBallCollision(vec3 ballPos, float ballRadius, vec3 ballVelocity);
    
    // 핀끼리 충돌 체크
    void CheckPinCollisions();
    
    // 렌더링
    void Draw();
    
    // 서있는 핀 수
    int CountStanding();
    
    // 모든 핀이 멈췄는지
    bool AllSettled();
};
