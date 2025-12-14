#pragma once
#include "Common.h"

class Ball {
public:
    // 기본 속성
    vec3 position;
    vec3 velocity;
    vec3 angularVelocity;
    float radius;
    float mass;

    // 회전 (렌더링용)
    float rotationAngle;
    vec3 rotationAxis;

    // 상태
    SpinType spinType;
    bool isRolling;
    bool isInGutter;
    int ballType;           // 0=빨강, 1=파랑, 2=초록

    // 스플라인 관련
    float rollTime;         // 굴린 후 경과 시간
    float totalTime;        // 전체 예상 시간
    vec3 splineP0;          // 제어점 0
    vec3 splineP1;          // 제어점 1
    vec3 splineP2;          // 제어점 2
    vec3 splineP3;          // 제어점 3
    bool useSpline;         // 스플라인 사용 여부

    // 생성자
    Ball();

    // 초기화
    void Reset();

    // 공 발사
    void Launch(float power, SpinType spin);

    // 업데이트
    void Update(float dt);
    void ApplyFriction(float dt);
    void CheckGutter();

    // 스플라인 함수
    void SetupSpline(float power, SpinType spin);
    vec3 EvaluateCardinalSpline(float t);

    // 렌더링
    void Draw();

    // 공 타입 설정
    void SetBallType(int type);

    // 시작 위치 설정하며 리셋
    void Reset(float startX);

    // 정지 여부 확인
    bool IsStopped();
};