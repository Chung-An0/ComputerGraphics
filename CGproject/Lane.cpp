#include "Lane.h"
#include "Texture.h"

Lane::Lane() {
    laneTexture = 0;
    gutterTexture = 0;
    foulLineTexture = 0;
    lightPosition = vec3(0.0f, 5.0f, -LANE_LENGTH / 2.0f);

    // 새 텍스처 초기화
    laneSurfaceTexture = 0;
    wallTexture = 0;
    ceilingTexture = 0;
    floorTexture = 0;
}

void Lane::Init() {
    // 텍스처 로딩: textures 폴더에 적절한 이미지 파일을 준비해야 합니다.
    // 예: lane.jpg, wall.jpg, ceiling.jpg, floor.jpg
    laneSurfaceTexture = Texture::Load("textures/lane.jpg");
    wallTexture = Texture::Load("textures/wall.jpg");
    ceilingTexture = Texture::Load("textures/ceiling.jpg");
    floorTexture = Texture::Load("textures/floor.jpg");
}

void Lane::Draw() {
    // 조명 설정
    GLfloat lightPos[] = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    DrawLaneSurface();
    DrawGutters();
    DrawFoulLine();
    DrawEnvironment();
    DrawPinArea();
}

void Lane::DrawLaneSurface() {
    // 레인 바닥 텍스처 적용
    float halfWidth = LANE_WIDTH / 2.0f;

    glEnable(GL_TEXTURE_2D);
    if (laneSurfaceTexture != 0) {
        Texture::Bind(laneSurfaceTexture, 0);
    }

    // 스펙큘러 및 광택 설정
    GLfloat specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat shininess[] = { 30.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    // 텍스처 좌표와 함께 레인 바닥
    // 앞쪽 (플레이어 방향) -> z = 3.0f, 뒤쪽 -> z = -LANE_LENGTH
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, 0.0f, 3.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(halfWidth, 0.0f, 3.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(halfWidth, 0.0f, -LANE_LENGTH);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-halfWidth, 0.0f, -LANE_LENGTH);
    glEnd();

    if (laneSurfaceTexture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);

    // 레인 판자 줄무늬 (어두운 선)은 텍스처가 없는 경우에만 그린다
    if (laneSurfaceTexture == 0) {
        GLfloat matStripe[] = { 0.6f, 0.45f, 0.3f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matStripe);
        glBegin(GL_LINES);
        float stripeSpacing = 0.1f;  // 판자 간격
        for (float x = -halfWidth; x <= halfWidth; x += stripeSpacing) {
            glVertex3f(x, 0.002f, 3.0f);
            glVertex3f(x, 0.002f, -LANE_LENGTH);
        }
        glEnd();
    }
}

void Lane::DrawGutters() {
    // 거터 색상 (어두운 회색)
    GLfloat matGutter[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matGutter);

    float laneEdge = LANE_WIDTH / 2.0f;
    float gutterOuter = laneEdge + GUTTER_WIDTH;
    float gutterDepth = -0.05f;  // 거터 깊이

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
    // 파울라인 (빨간색 또는 검은색)
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
    // 벽, 천장, 뒤쪽 벽, 바닥에 텍스처 적용
    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 0.5f;
    float wallHeight = 3.0f;
    float gutterOuter = LANE_WIDTH / 2.0f + GUTTER_WIDTH;

    // ----- 왼쪽/오른쪽 벽 -----
    glEnable(GL_TEXTURE_2D);
    if (wallTexture != 0) {
        Texture::Bind(wallTexture, 0);
    }
    glBegin(GL_QUADS);
    // 왼쪽 벽 (U: z 방향, V: 높이)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-wallDist, 0.0f, 5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-wallDist, wallHeight, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    // 오른쪽 벽
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(wallDist, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(wallDist, wallHeight, 5.0f);
    glEnd();
    if (wallTexture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);

    // ----- 천장 -----
    glEnable(GL_TEXTURE_2D);
    if (ceilingTexture != 0) {
        Texture::Bind(ceilingTexture, 0);
    }
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-wallDist, wallHeight, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(wallDist, wallHeight, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glEnd();
    if (ceilingTexture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);

    // ----- 뒤쪽 벽 (플레이어 뒤) -----
    glEnable(GL_TEXTURE_2D);
    if (wallTexture != 0) {
        Texture::Bind(wallTexture, 0);
    }
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-wallDist, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(wallDist, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(wallDist, wallHeight, 5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-wallDist, wallHeight, 5.0f);
    glEnd();
    if (wallTexture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);

    // ----- 바닥 (레인 외 영역) -----
    glEnable(GL_TEXTURE_2D);
    if (floorTexture != 0) {
        Texture::Bind(floorTexture, 0);
    }
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    // 왼쪽 바닥
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-wallDist, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-gutterOuter, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-gutterOuter, 0.0f, -LANE_LENGTH - 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    // 오른쪽 바닥
    glTexCoord2f(0.0f, 0.0f); glVertex3f(gutterOuter, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(wallDist, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(gutterOuter, 0.0f, -LANE_LENGTH - 2.0f);
    glEnd();
    if (floorTexture != 0) {
        Texture::Unbind();
    }
    glDisable(GL_TEXTURE_2D);
}

void Lane::DrawPinArea() {
    // 핀 영역 배경 (어두운 배경)
    GLfloat matPinArea[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matPinArea);

    float halfWidth = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 0.5f;

    // 핀 뒤쪽 벽
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-halfWidth, 0.0f, -LANE_LENGTH - 1.0f);
    glVertex3f(halfWidth, 0.0f, -LANE_LENGTH - 1.0f);
    glVertex3f(halfWidth, 2.0f, -LANE_LENGTH - 1.0f);
    glVertex3f(-halfWidth, 2.0f, -LANE_LENGTH - 1.0f);
    glEnd();
}
