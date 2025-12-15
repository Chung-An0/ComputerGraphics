#pragma once
#include "Common.h"
#include "Texture.h"
#include <vector>
#include <string>

class Lane {
public:
    // 텍스처 리스트 (여러 장 저장)
    vector<GLuint> laneTextures;
    vector<GLuint> wallTextures;
    vector<GLuint> ceilingTextures;
    vector<GLuint> floorTextures;

    // 현재 선택된 텍스처 인덱스 (0번이 기본)
    int currentLaneIndex;
    int currentWallIndex;
    int currentCeilingIndex;
    int currentFloorIndex;

    // 기타 텍스처
    GLuint gutterTexture;
    GLuint foulLineTexture;

    // 조명
    vec3 lightPosition;

    Lane();

    // 초기화
    void Init();

    // 렌더링
    void Draw();
    void DrawLaneSurface();
    void DrawGutters();
    void DrawFoulLine();
    void DrawEnvironment();
    void DrawPinArea();

    // 텍스처 변경 함수 (안전하게 범위 체크)
    void SetLaneTexture(int index);
    void SetWallTexture(int index);
    void SetCeilingTexture(int index);
    void SetFloorTexture(int index);
};