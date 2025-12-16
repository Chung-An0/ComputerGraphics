#pragma once
#include "Common.h"
#include "Camera.h"
#include "Ball.h"
#include "Pin.h"
#include "Lane.h"
#include "UI.h"

class Game {
public:
    // 게임 상태
    GameState state;
    
    // 게임 오브젝트
    Camera camera;
    Ball ball;
    PinManager pins;
    Lane lane;
    UI ui;
    
    // 타이머
    float deltaTime;
    int lastTime;
    float pinSettleTimer;       // 핀 안정화 대기 시간
    float transitionTimer;      // 상태 전환 대기 시간
    
    // 투구 결과
    int pinsKnockedThisThrow;
    bool showStrike;
    bool showSpare;
    float messageTimer;
    
    // 싱글톤
    static Game& Instance();
    
    // 초기화
    void Init();
    
    // 게임 루프
    void Update();
    void Render();
    
    // 상태 관리
    void SetState(GameState newState);
    void UpdateAiming();
    void UpdateCharging();
    void UpdateRolling();
    void UpdatePinAction();
    void UpdateFrameEnd();
    
    // 입력 처리
    void OnKeyDown(unsigned char key);
    void OnKeyUp(unsigned char key);
    void OnSpecialKey(int key);
    void OnMouse(int button, int state, int x, int y);
    void OnMouseMove(int x, int y);
    
    // 프레임 관리
    void NextThrow();
    void NextFrame();
    void ResetForThrow();
    void ResetGame();
    
    // 유틸리티
    void CalculateDeltaTime();
    
private:
    // 기본 생성자에서 멤버를 초기화합니다. 정의는 Game.cpp에서 제공합니다.
    Game();
    bool keys[256];             // 키 상태
    bool spacePressed;
};

// 전역 콜백 래퍼 함수
void DisplayCallback();
void ReshapeCallback(int w, int h);
void KeyboardCallback(unsigned char key, int x, int y);
void KeyboardUpCallback(unsigned char key, int x, int y);
void SpecialCallback(int key, int x, int y);
void MouseCallback(int button, int state, int x, int y);
void MotionCallback(int x, int y);
void PassiveMotionCallback(int x, int y);
void TimerCallback(int value);
