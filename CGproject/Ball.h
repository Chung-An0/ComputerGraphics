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
};
