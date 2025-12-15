#include "Lane.h"

Lane::Lane() {
    laneTexture = 0;
    gutterTexture = 0;
    foulLineTexture = 0;
    lightPosition = vec3(0.0f, 5.0f, -LANE_LENGTH / 2.0f);
}

void Lane::Init() {
    // 텍스처 로딩은 나중에 (없어도 색상으로 표현)
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
    // 레인 바닥 (나무 색상)
    GLfloat matLane[] = { 0.87f, 0.72f, 0.53f, 1.0f };  // 밝은 나무색
    GLfloat matSpecular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat matShininess[] = { 30.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matLane);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    float halfWidth = LANE_WIDTH / 2.0f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    // 레인 바닥 (파울라인 앞 ~ 핀 영역)
    glVertex3f(-halfWidth, 0.0f, 3.0f);            // 시작점 (플레이어 뒤)
    glVertex3f(halfWidth, 0.0f, 3.0f);
    glVertex3f(halfWidth, 0.0f, -LANE_LENGTH);
    glVertex3f(-halfWidth, 0.0f, -LANE_LENGTH);

    glEnd();

    // 레인 판자 줄무늬 (어두운 선)
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
    // 양쪽 벽
    GLfloat matWall[] = { 0.4f, 0.35f, 0.3f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWall);

    float wallDist = LANE_WIDTH / 2.0f + GUTTER_WIDTH + 0.5f;
    float wallHeight = 3.0f;

    glBegin(GL_QUADS);

    // 왼쪽 벽
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-wallDist, 0.0f, 5.0f);
    glVertex3f(-wallDist, wallHeight, 5.0f);
    glVertex3f(-wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glVertex3f(-wallDist, 0.0f, -LANE_LENGTH - 2.0f);

    // 오른쪽 벽
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(wallDist, 0.0f, 5.0f);
    glVertex3f(wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    glVertex3f(wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glVertex3f(wallDist, wallHeight, 5.0f);

    glEnd();

    // 천장
    GLfloat matCeiling[] = { 0.3f, 0.3f, 0.35f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matCeiling);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-wallDist, wallHeight, 5.0f);
    glVertex3f(wallDist, wallHeight, 5.0f);
    glVertex3f(wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glVertex3f(-wallDist, wallHeight, -LANE_LENGTH - 2.0f);
    glEnd();

    // 뒤쪽 벽 (플레이어 뒤)
    GLfloat matBackWall[] = { 0.5f, 0.45f, 0.4f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matBackWall);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-wallDist, 0.0f, 5.0f);
    glVertex3f(wallDist, 0.0f, 5.0f);
    glVertex3f(wallDist, wallHeight, 5.0f);
    glVertex3f(-wallDist, wallHeight, 5.0f);
    glEnd();

    // 바닥 (레인 외 영역)
    GLfloat matFloor[] = { 0.3f, 0.25f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matFloor);

    float gutterOuter = LANE_WIDTH / 2.0f + GUTTER_WIDTH;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    // 왼쪽 바닥
    glVertex3f(-wallDist, 0.0f, 5.0f);
    glVertex3f(-gutterOuter, 0.0f, 5.0f);
    glVertex3f(-gutterOuter, 0.0f, -LANE_LENGTH - 2.0f);
    glVertex3f(-wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    // 오른쪽 바닥
    glVertex3f(gutterOuter, 0.0f, 5.0f);
    glVertex3f(wallDist, 0.0f, 5.0f);
    glVertex3f(wallDist, 0.0f, -LANE_LENGTH - 2.0f);
    glVertex3f(gutterOuter, 0.0f, -LANE_LENGTH - 2.0f);
    glEnd();
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
