#pragma once

#define _CRT_SECURE_NO_WARNINGS

// OpenGL 관련
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM 수학 라이브러리
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 표준 라이브러리
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// 네임스페이스
using namespace std;
using namespace glm;

// ========== 게임 상수 ==========

// 윈도우 크기
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// 볼링 레인 크기 (실제 비율 기준)
const float LANE_LENGTH = 18.29f;    // 약 60피트
const float LANE_WIDTH = 1.05f;      // 약 41.5인치
const float GUTTER_WIDTH = 0.15f;    // 거터 폭
const float FOUL_LINE_Z = 0.0f;      // 파울라인 위치

// 핀 관련
const float PIN_HEIGHT = 0.38f;      // 핀 높이 (약 15인치)
const float PIN_RADIUS = 0.057f;     // 핀 반지름
const float PIN_START_Z = -LANE_LENGTH + 1.0f;  // 핀 시작 위치
const float PIN_SPACING = 0.30f;     // 핀 간격

// 공 관련
const float BALL_RADIUS = 0.11f;     // 공 반지름 (약 8.5인치)
const float BALL_MASS = 6.0f;        // 공 질량 (kg)

// 물리 상수
const float GRAVITY = -9.8f;
const float FRICTION = 0.02f;        // 레인 마찰
const float RESTITUTION = 0.7f;      // 반발 계수

// 게임 상태
enum class GameState {
    AIMING,         // 조준 중 (WASD로 위치 조정)
    CHARGING,       // 파워 게이지 차는 중
    ROLLING,        // 공 굴러가는 중
    PIN_ACTION,     // 핀 쓰러지는 중
    FRAME_END,      // 프레임 종료
    GAME_OVER       // 게임 종료
};

// 스핀 타입
enum class SpinType {
    STRAIGHT,       // 직구
    LEFT_HOOK,      // 왼쪽 스핀
    RIGHT_HOOK      // 오른쪽 스핀
};

// 카메라 모드
enum class CameraMode {
    FIRST_PERSON,   // 1인칭 (플레이어 시점)
    BALL_FOLLOW     // 공 추적
};
