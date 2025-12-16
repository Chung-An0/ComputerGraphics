#include "Lane.h"

Lane::Lane() {
    // 초기 인덱스는 0번(첫 번째 텍스처)
    currentLaneIndex = 0;
    currentWallIndex = 0;
    currentCeilingIndex = 0;
    currentFloorIndex = 0;

    gutterTexture = 0;
    foulLineTexture = 0;
    lightPosition = vec3(0.0f, 5.0f, -LANE_LENGTH / 2.0f);
}

void Lane::Init() {
    // 각 카테고리별 텍스처 로딩 (1.jpg ~ 5.jpg)
    // 만약 파일이 없으면 Texture::Load 내부에서 오류 메시지를 내고 0을 반환
    for (int i = 1; i <= 5; i++) {
        string num = to_string(i);

        // Lane 텍스처 로드
        string lanePath = "Textures/Lane/" + num + ".jpg";
        GLuint laneTex = Texture::Load(lanePath.c_str());
        if (laneTex != 0) {
            laneTextures.push_back(laneTex);
        }

        // Wall 텍스처 로드
        string wallPath = "Textures/Wall/" + num + ".jpg";
        GLuint wallTex = Texture::Load(wallPath.c_str());
        if (wallTex != 0) {
            wallTextures.push_back(wallTex);
        }

        // Ceiling 텍스처 로드
        string ceilingPath = "Textures/Ceiling/" + num + ".jpg";
        GLuint ceilingTex = Texture::Load(ceilingPath.c_str());
        if (ceilingTex != 0) {
            ceilingTextures.push_back(ceilingTex);
        }

        // Floor 텍스처 로드
        string floorPath = "Textures/Floor/" + num + ".jpg";
        GLuint floorTex = Texture::Load(floorPath.c_str());
        if (floorTex != 0) {
            floorTextures.push_back(floorTex);
        }
    }

    // 텍스처 로딩 실패 시 벡터가 비어있을 수 있으므로 인덱스 체크 필요
    if (!laneTextures.empty() && currentLaneIndex >= laneTextures.size()) {
        currentLaneIndex = 0;
    }
    if (!wallTextures.empty() && currentWallIndex >= wallTextures.size()) {
        currentWallIndex = 0;
    }
    if (!ceilingTextures.empty() && currentCeilingIndex >= ceilingTextures.size()) {
        currentCeilingIndex = 0;
    }
    if (!floorTextures.empty() && currentFloorIndex >= floorTextures.size()) {
        currentFloorIndex = 0;
    }
}

void Lane::Draw() {
    // 조명 설정 (1번, 2번 공통)
    GLfloat lightPos[] = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // 텍스처 환경 모드 설정
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    DrawLaneSurface();
    DrawGutters();
    DrawFoulLine();
    DrawEnvironment();
    DrawPinArea();
}

void Lane::DrawLaneSurface() {
    // 2번 코드 기반 - 텍스처 시스템 사용
    // 1번의 스펙큘러 설정 통합
    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matSpecular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat matShininess[] = { 30.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    float halfWidth = LANE_WIDTH / 2.0f;
    float repeatV = 10.0f; // 텍스처 반복 (세로 방향)

    // 텍스처가 로드된 경우에만 적용
    if (!laneTextures.empty() && currentLaneIndex >= 0 && currentLaneIndex < laneTextures.size()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, laneTextures[currentLaneIndex]);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, 0.0f, 3.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(halfWidth, 0.0f, 3.0f);
        glTexCoord2f(1.0f, repeatV); glVertex3f(halfWidth, 0.0f, -LANE_LENGTH);
        glTexCoord2f(0.0f, repeatV); glVertex3f(-halfWidth, 0.0f, -LANE_LENGTH);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        // 텍스처가 없을 때 - 1번 코드의 폴백 (판자 무늬)
        GLfloat matWood[] = { 0.7f, 0.5f, 0.3f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWood);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-halfWidth, 0.0f, 3.0f);
        glVertex3f(halfWidth, 0.0f, 3.0f);
        glVertex3f(halfWidth, 0.0f, -LANE_LENGTH);
        glVertex3f(-halfWidth, 0.0f, -LANE_LENGTH);
        glEnd();

        // 레인 판자 줄무늬
        GLfloat matStripe[] = { 0.6f, 0.45f, 0.3f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matStripe);
        glBegin(GL_LINES);
        float stripeSpacing = 0.1f;
        for (float x = -halfWidth; x <= halfWidth; x += stripeSpacing) {
            glVertex3f(x, 0.002f, 3.0f);
            glVertex3f(x, 0.002f, -LANE_LENGTH);
        }
        glEnd();
    }
}

void Lane::DrawGutters() {
    // 1번 코드 기반 - 거터 그리기
    GLfloat matGutter[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matGutter);

    float laneEdge = LANE_WIDTH / 2.0f;
    float gutterOuter = laneEdge + GUTTER_WIDTH;
    float gutterDepth = -0.05f;

    // 왼쪽 거터
    glBegin(GL_QUADS);

    // 바닥
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-gutterOuter, gutterDepth, 0.0f);
    glVertex3f(-laneEdge, gutterDepth, 0.0f);
    glVertex3f(-laneEdge, gutterDepth, -LANE_LENGTH);
    glVertex3f(-gutterOuter, gutterDepth, -LANE_LENGTH);

    // 안쪽 벽
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-laneEdge, 0.0f, 0.0f);
    glVertex3f(-laneEdge, gutterDepth, 0.0f);
    glVertex3f(-laneEdge, gutterDepth, -LANE_LENGTH);
    glVertex3f(-laneEdge, 0.0f, -LANE_LENGTH);

    glEnd();

    // 오른쪽 거터
    glBegin(GL_QUADS);

    // 바닥
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(laneEdge, gutterDepth, 0.0f);
    glVertex3f(gutterOuter, gutterDepth, 0.0f);
    glVertex3f(gutterOuter, gutterDepth, -LANE_LENGTH);
    glVertex3f(laneEdge, gutterDepth, -LANE_LENGTH);

    // 안쪽 벽
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(laneEdge, 0.0f, 0.0f);
    glVertex3f(laneEdge, 0.0f, -LANE_LENGTH);
    glVertex3f(laneEdge, gutterDepth, -LANE_LENGTH);
    glVertex3f(laneEdge, gutterDepth, 0.0f);

    glEnd();
}

void Lane::DrawFoulLine() {
    // 1번 코드 기반 - 파울라인
    GLfloat matFoul[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matFoul);

    float halfWidth = LANE_WIDTH / 2.0f + GUTTER_WIDTH;
    float lineWidth = 0.03f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-halfWidth, 0.003f, FOUL_LINE_Z + lineWidth);
    glVertex3f(halfWidth, 0.003f, FOUL_LINE_Z + lineWidth);
    glVertex3f(halfWidth, 0.003f, FOUL_LINE_Z - lineWidth);
    glVertex3f(-halfWidth, 0.003f, FOUL_LINE_Z - lineWidth);
    glEnd();
}

void Lane::DrawEnvironment() {
    // 2번 코드 기반 - 벽, 천장, 바닥 텍스처 시스템
    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);

    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 5.0f;
    float wallHeight = 3.0f;
    float startZ = 5.0f;
    float endZ = -LANE_LENGTH - 2.0f;
    float repeatX = 5.0f;
    float repeatZ = 5.0f;
    float gutterOuter = LANE_WIDTH / 2.0f + GUTTER_WIDTH;

    // ============ 벽 (Walls) ============
    if (!wallTextures.empty() && currentWallIndex >= 0 && currentWallIndex < wallTextures.size()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, wallTextures[currentWallIndex]);

        glBegin(GL_QUADS);
        // 왼쪽 벽
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);    glVertex3f(-wallDist, 0.0f, startZ);
        glTexCoord2f(repeatZ, 0.0f); glVertex3f(-wallDist, 0.0f, endZ);
        glTexCoord2f(repeatZ, 1.0f); glVertex3f(-wallDist, wallHeight, endZ);
        glTexCoord2f(0.0f, 1.0f);    glVertex3f(-wallDist, wallHeight, startZ);

        // 오른쪽 벽
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);    glVertex3f(wallDist, 0.0f, endZ);
        glTexCoord2f(repeatZ, 0.0f); glVertex3f(wallDist, 0.0f, startZ);
        glTexCoord2f(repeatZ, 1.0f); glVertex3f(wallDist, wallHeight, startZ);
        glTexCoord2f(0.0f, 1.0f);    glVertex3f(wallDist, wallHeight, endZ);

        // 뒤쪽 벽
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);    glVertex3f(-wallDist, 0.0f, startZ);
        glTexCoord2f(repeatX, 0.0f); glVertex3f(wallDist, 0.0f, startZ);
        glTexCoord2f(repeatX, 1.0f); glVertex3f(wallDist, wallHeight, startZ);
        glTexCoord2f(0.0f, 1.0f);    glVertex3f(-wallDist, wallHeight, startZ);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        // 텍스처 없을 때 - 기본 색상
        GLfloat matWall[] = { 0.5f, 0.5f, 0.6f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWall);

        glBegin(GL_QUADS);
        // 왼쪽 벽
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-wallDist, 0.0f, startZ);
        glVertex3f(-wallDist, 0.0f, endZ);
        glVertex3f(-wallDist, wallHeight, endZ);
        glVertex3f(-wallDist, wallHeight, startZ);

        // 오른쪽 벽
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(wallDist, 0.0f, endZ);
        glVertex3f(wallDist, 0.0f, startZ);
        glVertex3f(wallDist, wallHeight, startZ);
        glVertex3f(wallDist, wallHeight, endZ);

        // 뒤쪽 벽
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-wallDist, 0.0f, startZ);
        glVertex3f(wallDist, 0.0f, startZ);
        glVertex3f(wallDist, wallHeight, startZ);
        glVertex3f(-wallDist, wallHeight, startZ);
        glEnd();
    }

    // ============ 천장 (Ceiling) ============
    if (!ceilingTextures.empty() && currentCeilingIndex >= 0 && currentCeilingIndex < ceilingTextures.size()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, ceilingTextures[currentCeilingIndex]);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);        glVertex3f(-wallDist, wallHeight, startZ);
        glTexCoord2f(repeatX, 0.0f);     glVertex3f(wallDist, wallHeight, startZ);
        glTexCoord2f(repeatX, repeatZ);  glVertex3f(wallDist, wallHeight, endZ);
        glTexCoord2f(0.0f, repeatZ);     glVertex3f(-wallDist, wallHeight, endZ);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        // 텍스처 없을 때 - 기본 색상
        GLfloat matCeiling[] = { 0.7f, 0.7f, 0.7f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matCeiling);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-wallDist, wallHeight, startZ);
        glVertex3f(wallDist, wallHeight, startZ);
        glVertex3f(wallDist, wallHeight, endZ);
        glVertex3f(-wallDist, wallHeight, endZ);
        glEnd();
    }

    // ============ 바닥 (Floor - 레인 외곽) ============
    if (!floorTextures.empty() && currentFloorIndex >= 0 && currentFloorIndex < floorTextures.size()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, floorTextures[currentFloorIndex]);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        // 왼쪽 바닥
        glTexCoord2f(0.0f, 0.0f);        glVertex3f(-wallDist, 0.0f, startZ);
        glTexCoord2f(repeatX / 2, 0.0f);   glVertex3f(-gutterOuter, 0.0f, startZ);
        glTexCoord2f(repeatX / 2, repeatZ); glVertex3f(-gutterOuter, 0.0f, endZ);
        glTexCoord2f(0.0f, repeatZ);     glVertex3f(-wallDist, 0.0f, endZ);
        // 오른쪽 바닥
        glTexCoord2f(0.0f, 0.0f);        glVertex3f(gutterOuter, 0.0f, startZ);
        glTexCoord2f(repeatX / 2, 0.0f);   glVertex3f(wallDist, 0.0f, startZ);
        glTexCoord2f(repeatX / 2, repeatZ); glVertex3f(wallDist, 0.0f, endZ);
        glTexCoord2f(0.0f, repeatZ);     glVertex3f(gutterOuter, 0.0f, endZ);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        // 텍스처 없을 때 - 기본 색상
        GLfloat matFloor[] = { 0.3f, 0.3f, 0.35f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matFloor);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        // 왼쪽 바닥
        glVertex3f(-wallDist, 0.0f, startZ);
        glVertex3f(-gutterOuter, 0.0f, startZ);
        glVertex3f(-gutterOuter, 0.0f, endZ);
        glVertex3f(-wallDist, 0.0f, endZ);
        // 오른쪽 바닥
        glVertex3f(gutterOuter, 0.0f, startZ);
        glVertex3f(wallDist, 0.0f, startZ);
        glVertex3f(wallDist, 0.0f, endZ);
        glVertex3f(gutterOuter, 0.0f, endZ);
        glEnd();
    }
}

void Lane::DrawPinArea() {
    // 2번 코드 기반 - 핀 영역 뒤쪽 벽 (텍스처 적용)
    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);

    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 5.0f;
    float wallHeight = 3.0f;
    float repeatX = 5.0f;

    if (!wallTextures.empty() && currentWallIndex >= 0 && currentWallIndex < wallTextures.size()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, wallTextures[currentWallIndex]);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(repeatX, 0.0f); glVertex3f(-wallDist, 0.0f, -LANE_LENGTH - 1.0f);
        glTexCoord2f(0.0f, 0.0f);    glVertex3f(wallDist, 0.0f, -LANE_LENGTH - 1.0f);
        glTexCoord2f(0.0f, 1.0f);    glVertex3f(wallDist, wallHeight, -LANE_LENGTH - 1.0f);
        glTexCoord2f(repeatX, 1.0f); glVertex3f(-wallDist, wallHeight, -LANE_LENGTH - 1.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        // 텍스처 없을 때 - 1번 코드의 어두운 배경
        GLfloat matPinArea[] = { 0.15f, 0.15f, 0.15f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matPinArea);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-wallDist, 0.0f, -LANE_LENGTH - 1.0f);
        glVertex3f(wallDist, 0.0f, -LANE_LENGTH - 1.0f);
        glVertex3f(wallDist, 2.0f, -LANE_LENGTH - 1.0f);
        glVertex3f(-wallDist, 2.0f, -LANE_LENGTH - 1.0f);
        glEnd();
    }
}

// 텍스처 인덱스 변경 함수 (범위 체크)
void Lane::SetLaneTexture(int index) {
    if (index >= 0 && index < laneTextures.size()) {
        currentLaneIndex = index;
    }
}

void Lane::SetWallTexture(int index) {
    if (index >= 0 && index < wallTextures.size()) {
        currentWallIndex = index;
    }
}

void Lane::SetCeilingTexture(int index) {
    if (index >= 0 && index < ceilingTextures.size()) {
        currentCeilingIndex = index;
    }
}

void Lane::SetFloorTexture(int index) {
    if (index >= 0 && index < floorTextures.size()) {
        currentFloorIndex = index;
    }
}