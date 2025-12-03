#pragma once
#include "Common.h"

class Ball {
public:
    // 물리 속성
    vec3 position;
    vec3 velocity;
    vec3 angularVelocity;   // 스핀 (각속도)
    float radius;
    float mass;
    
    // 회전 상태 (렌더링용)
    float rotationAngle;
    vec3 rotationAxis;
    
    // 스핀 타입
    SpinType spinType;
    
    // 상태
    bool isRolling;
    bool isInGutter;
    
    // 텍스처/색상
    GLuint textureID;
    vec3 color;
    int ballType;   // 공 종류 (0, 1, 2...)
    
    // 생성자
    Ball();
    
    // 초기화 (새 프레임 시작)
    void Reset(float startX);
    
    // 물리 업데이트
    void Update(float dt);
    
    // 공 발사
    void Launch(float power, SpinType spin);
    
    // 스핀 효과 적용 (마그누스 효과)
    void ApplySpinEffect(float dt);
    
    // 거터 체크
    void CheckGutter();
    
    // 레인 마찰 적용
    void ApplyFriction(float dt);
    
    // 렌더링
    void Draw();
    
    // 공 종류 변경
    void SetBallType(int type);
    
    // 정지 여부
    bool IsStopped();
};
