#pragma once
#include "Common.h"

// 프레임 정보
struct Frame {
    int firstThrow;
    int secondThrow;
    int totalScore;     // 누적 점수
    bool isStrike;
    bool isSpare;
    bool isComplete;
};

// 스핀 버튼 영역
struct Button {
    float x, y, width, height;
    const char* label;
    SpinType spinType;
    bool isHovered;
    bool isSelected;
};

class UI {
public:
    // 게이지
    float powerGauge;           // 0.0 ~ 1.0
    bool isCharging;
    float gaugeSpeed;

    // 메뉴 상태
    bool menuOpen;
    int selectedBall;           // 0, 1, 2
    int previewBall;            // 메뉴에서 보여주는 공
    SpinType selectedSpin;

    // 스핀 버튼들
    Button spinButtons[3];

    // 점수판
    Frame frames[10];
    int currentFrame;
    int currentThrow;           // 1 or 2

    // 카메라 pitch (점수판 보이는지 체크용)
    float cameraPitch;

    // 생성자
    UI();

    // 초기화
    void Reset();

    // 게이지 업데이트
    void UpdateGauge(float dt);
    void StartCharging();
    float StopCharging();       // 현재 파워 반환

    // 점수 계산
    void RecordThrow(int pinsKnocked);
    int CalculateTotalScore();

    // 렌더링
    void Draw();
    void DrawPowerGauge();
    void DrawScoreboard3D();    // 3D 천장 점수판
    void DrawMenu();            // 중앙 메뉴
    void DrawSpinButtons();     // 하단 스핀 버튼
    void DrawBallPreview();     // 메뉴 내 공 미리보기

    // 메뉴 조작
    void ToggleMenu();
    void MenuLeft();            // 방향키 왼쪽
    void MenuRight();           // 방향키 오른쪽
    void SelectBall();          // 공 선택 확정

    // 마우스 처리
    void OnMouseClick(int x, int y);
    void OnMouseMove(int x, int y);
    bool IsPointInButton(int x, int y, Button& btn);

    // 텍스트 렌더링 헬퍼
    void DrawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18);
    void DrawTextLarge(float x, float y, const char* text);
    void DrawTextCentered(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18);

    // 2D 모드 전환
    void Begin2D();
    void End2D();

    // 게임 상태 표시
    void DrawGameState(GameState state);
    void DrawStrikeSpare(bool isStrike, bool isSpare);

    // 카메라 pitch 업데이트 (점수판 가시성)
    void SetCameraPitch(float pitch);
};