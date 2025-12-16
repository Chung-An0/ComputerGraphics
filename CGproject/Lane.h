#pragma once
#include "Common.h"

class Lane {
public:
    // 텍스처
    GLuint laneTexture;
    GLuint gutterTexture;
    GLuint foulLineTexture;

    // 새 텍스처: 레인 표면, 벽, 천장, 바닥
    GLuint laneSurfaceTexture;
    GLuint wallTexture;
    GLuint ceilingTexture;
    GLuint floorTexture;

    // 조명
    vec3 lightPosition;

    Lane();

    // 초기화
    void Init();

    // 렌더링
    void Draw();

    // 레인 바닥
    void DrawLaneSurface();

    // 거터
    void DrawGutters();

    // 파울라인
    void DrawFoulLine();

    // 주변 환경 (벽, 천장 등)
    void DrawEnvironment();

    // 핀 영역 배경
    void DrawPinArea();
};
