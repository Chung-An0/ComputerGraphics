#pragma once
#include "Common.h"

class Camera {
public:
    // 현재 모드
    CameraMode mode;
    
    // 1인칭 카메라 속성
    vec3 position;          // 카메라 위치
    float pitch;            // 상하 각도 (고개 들기/숙이기)
    float yaw;              // 좌우 각도
    
    // 공 추적 카메라 속성
    vec3 targetPosition;    // 추적 대상 (공) 위치
    vec3 targetDirection;   // 공 진행 방향
    float followDistance;   // 공과의 거리
    float followHeight;     // 공 위 높이
    
    // 플레이어 좌우 위치 (A/D로 조정)
    float playerX;
    
    // 생성자
    Camera();
    
    // 뷰 행렬 적용
    void Apply();
    
    // 1인칭 모드 업데이트
    void UpdateFirstPerson();
    
    // 공 추적 모드 업데이트
    void UpdateBallFollow(vec3 ballPos, vec3 ballDir);
    
    // 입력 처리
    void LookUp(float amount);      // W: 고개 들기
    void LookDown(float amount);    // S: 고개 숙이기
    void MoveLeft(float amount);    // A: 왼쪽 이동
    void MoveRight(float amount);   // D: 오른쪽 이동
    
    // 모드 전환
    void SetMode(CameraMode newMode);
    
    // 시점 방향 벡터
    vec3 GetForward();
    vec3 GetRight();
    vec3 GetUp();
};
