#include "Lane.h"

Lane::Lane() {
    // 초기 인덱스는 0번(첫 번째 사진)
    currentLaneIndex = 0;
    currentWallIndex = 0;
    currentCeilingIndex = 0;
    currentFloorIndex = 0;

    gutterTexture = 0;
    foulLineTexture = 0;
    lightPosition = vec3(0.0f, 5.0f, -LANE_LENGTH / 2.0f);
}

void Lane::Init() {
    // 각 구역별로 3장씩 텍스처를 로드한다고 가정 (1.jpg ~ 3.jpg)
    // 파일이 없으면 Texture::Load 내부에서 에러 메시지를 띄우고 검은색 텍스처가 됨
    for (int i = 1; i <= 5; i++) {
        string num = to_string(i);

        // Lane 텍스처 로드
        string lanePath = "Textures/Lane/" + num + ".jpg";
        laneTextures.push_back(Texture::Load(lanePath.c_str()));

        // Wall 텍스처 로드
        string wallPath = "Textures/Wall/" + num + ".jpg";
        wallTextures.push_back(Texture::Load(wallPath.c_str()));

        // Ceiling 텍스처 로드
        string ceilingPath = "Textures/Ceiling/" + num + ".jpg";
        ceilingTextures.push_back(Texture::Load(ceilingPath.c_str()));

        // Floor 텍스처 로드
        string floorPath = "Textures/Floor/" + num + ".jpg";
        floorTextures.push_back(Texture::Load(floorPath.c_str()));
    }
}

void Lane::Draw() {
    // 조명 설정 (기존과 동일)
    GLfloat lightPos[] = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    DrawLaneSurface();
    DrawGutters();
    DrawFoulLine();
    DrawEnvironment();
    DrawPinArea();
}

void Lane::DrawLaneSurface() {
    if (laneTextures.empty()) return;

    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matSpecular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat matShininess[] = { 30.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    float halfWidth = LANE_WIDTH / 2.0f;
    float repeatV = 10.0f;

    glEnable(GL_TEXTURE_2D);
    // [변경] 현재 인덱스의 텍스처 바인딩
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

void Lane::DrawEnvironment() {
    if (wallTextures.empty() || ceilingTextures.empty() || floorTextures.empty()) return;

    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);

    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 5.0f;
    float wallHeight = 3.0f;
    float startZ = 5.0f;
    float endZ = -LANE_LENGTH - 2.0f;
    float repeatX = 5.0f;
    float repeatZ = 5.0f;

    glEnable(GL_TEXTURE_2D);

    // ============ 벽 (Walls) ============
    // [변경] 현재 인덱스의 벽 텍스처 바인딩
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

    // ============ 천장 (Ceiling) ============
    // [변경] 현재 인덱스의 천장 텍스처 바인딩
    glBindTexture(GL_TEXTURE_2D, ceilingTextures[currentCeilingIndex]);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);        glVertex3f(-wallDist, wallHeight, startZ);
    glTexCoord2f(repeatX, 0.0f);     glVertex3f(wallDist, wallHeight, startZ);
    glTexCoord2f(repeatX, repeatZ);  glVertex3f(wallDist, wallHeight, endZ);
    glTexCoord2f(0.0f, repeatZ);     glVertex3f(-wallDist, wallHeight, endZ);
    glEnd();

    // ============ 바닥 (Floor) ============
    // [변경] 현재 인덱스의 바닥 텍스처 바인딩
    glBindTexture(GL_TEXTURE_2D, floorTextures[currentFloorIndex]);
    float gutterOuter = LANE_WIDTH / 2.0f + GUTTER_WIDTH;

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

void Lane::DrawPinArea() {
    if (wallTextures.empty()) return;

    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);

    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 5.0f;
    float wallHeight = 3.0f;
    float repeatX = 5.0f;

    glEnable(GL_TEXTURE_2D);
    // [변경] 벽 텍스처 공유
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

// 텍스처 인덱스 변경 헬퍼 함수들 구현
void Lane::SetLaneTexture(int index) {
    if (index >= 0 && index < laneTextures.size()) currentLaneIndex = index;
}
void Lane::SetWallTexture(int index) {
    if (index >= 0 && index < wallTextures.size()) currentWallIndex = index;
}
void Lane::SetCeilingTexture(int index) {
    if (index >= 0 && index < ceilingTextures.size()) currentCeilingIndex = index;
}
void Lane::SetFloorTexture(int index) {
    if (index >= 0 && index < floorTextures.size()) currentFloorIndex = index;
}

// DrawGutters, DrawFoulLine은 이전과 동일하므로 생략하거나 그대로 둡니다.
void Lane::DrawGutters() { /* 이전 코드 유지 */ }
void Lane::DrawFoulLine() { /* 이전 코드 유지 */ }