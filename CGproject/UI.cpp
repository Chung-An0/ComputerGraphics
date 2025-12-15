// Visual Studio 보안 경고 비활성화
#define _CRT_SECURE_NO_WARNINGS

#include "UI.h"
#include "Game.h" // Game 인스턴스 접근을 위해 필요
#include <cstdio>
#include <cstring>

UI::UI() {
    // 스핀 버튼 초기화 (하단 중앙)
    float btnWidth = 100.0f;
    float btnHeight = 40.0f;
    float btnY = 30.0f;
    float startX = WINDOW_WIDTH / 2 - (btnWidth * 1.5f + 20);

    spinButtons[0] = { startX, btnY, btnWidth, btnHeight, "Straight", SpinType::STRAIGHT, false, true };
    spinButtons[1] = { startX + btnWidth + 10, btnY, btnWidth, btnHeight, "Left", SpinType::LEFT_HOOK, false, false };
    spinButtons[2] = { startX + (btnWidth + 10) * 2, btnY, btnWidth, btnHeight, "Right", SpinType::RIGHT_HOOK, false, false };

    Reset();
}

void UI::Reset() {
    powerGauge = 0.0f;
    isCharging = false;
    gaugeSpeed = 1.5f;

    menuOpen = false;
    selectedBall = 0;
    previewBall = 0;
    selectedSpin = SpinType::STRAIGHT;

    // [추가] 메뉴 카테고리 및 미리보기 초기화
    menuCategory = 0; // 0: 공 선택 메뉴부터 시작
    previewLaneTex = 0;
    previewWallTex = 0;
    previewCeilingTex = 0;
    previewFloorTex = 0;

    // 스핀 버튼 상태 리셋
    spinButtons[0].isSelected = true;
    spinButtons[1].isSelected = false;
    spinButtons[2].isSelected = false;

    currentFrame = 0;
    currentThrow = 1;
    cameraPitch = 0.0f;

    for (int i = 0; i < 10; i++) {
        frames[i].firstThrow = 0;
        frames[i].secondThrow = 0;
        frames[i].totalScore = 0;
        frames[i].isStrike = false;
        frames[i].isSpare = false;
        frames[i].isComplete = false;
    }
}

void UI::UpdateGauge(float dt) {
    if (!isCharging) return;

    powerGauge += gaugeSpeed * dt;

    if (powerGauge > 1.0f) {
        powerGauge = 1.0f;
        gaugeSpeed = -abs(gaugeSpeed);
    }
    else if (powerGauge < 0.0f) {
        powerGauge = 0.0f;
        gaugeSpeed = abs(gaugeSpeed);
    }
}

void UI::StartCharging() {
    isCharging = true;
    powerGauge = 0.0f;
    gaugeSpeed = 1.5f;
}

float UI::StopCharging() {
    isCharging = false;
    float power = powerGauge;
    powerGauge = 0.0f;
    return power;
}

void UI::RecordThrow(int pinsKnocked) {
    if (currentFrame >= 10) return;

    Frame& frame = frames[currentFrame];

    if (currentThrow == 1) {
        frame.firstThrow = pinsKnocked;

        if (pinsKnocked == 10) {
            frame.isStrike = true;
            frame.isComplete = true;
            currentFrame++;
            currentThrow = 1;
        }
        else {
            currentThrow = 2;
        }
    }
    else {
        frame.secondThrow = pinsKnocked;

        if (frame.firstThrow + frame.secondThrow == 10) {
            frame.isSpare = true;
        }

        frame.isComplete = true;
        currentFrame++;
        currentThrow = 1;
    }

    CalculateTotalScore();
}

int UI::CalculateTotalScore() {
    int total = 0;

    for (int i = 0; i < 10 && i <= currentFrame; i++) {
        Frame& frame = frames[i];

        if (!frame.isComplete) {
            frame.totalScore = total;
            continue;
        }

        int frameScore = frame.firstThrow + frame.secondThrow;

        if (frame.isStrike && i < 9) {
            if (i + 1 < 10) {
                frameScore += frames[i + 1].firstThrow;
                if (frames[i + 1].isStrike && i + 2 < 10) {
                    frameScore += frames[i + 2].firstThrow;
                }
                else {
                    frameScore += frames[i + 1].secondThrow;
                }
            }
        }
        else if (frame.isSpare && i < 9) {
            if (i + 1 < 10) {
                frameScore += frames[i + 1].firstThrow;
            }
        }

        total += frameScore;
        frame.totalScore = total;
    }

    return total;
}

void UI::Draw() {
    // 스핀 버튼 (항상 표시)
    Begin2D();
    DrawSpinButtons();
    DrawPowerGauge();
    End2D();

    // 메뉴 (M키로 열기)
    if (menuOpen) {
        Begin2D();
        DrawMenu();
        End2D();
    }
}

void UI::Begin2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

void UI::End2D() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UI::DrawPowerGauge() {
    // 오른쪽 중앙에 세로 게이지
    float gaugeX = WINDOW_WIDTH - 80;
    float gaugeY = WINDOW_HEIGHT / 2 - 150;
    float gaugeWidth = 30;
    float gaugeHeight = 300;

    // 배경
    glColor4f(0.2f, 0.2f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(gaugeX, gaugeY);
    glVertex2f(gaugeX + gaugeWidth, gaugeY);
    glVertex2f(gaugeX + gaugeWidth, gaugeY + gaugeHeight);
    glVertex2f(gaugeX, gaugeY + gaugeHeight);
    glEnd();

    // 테두리
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(gaugeX, gaugeY);
    glVertex2f(gaugeX + gaugeWidth, gaugeY);
    glVertex2f(gaugeX + gaugeWidth, gaugeY + gaugeHeight);
    glVertex2f(gaugeX, gaugeY + gaugeHeight);
    glEnd();

    // 게이지 바
    float fillHeight = powerGauge * gaugeHeight;

    glBegin(GL_QUADS);
    if (powerGauge < 0.5f) {
        glColor3f(0.0f, 1.0f, 0.0f);
    }
    else if (powerGauge < 0.8f) {
        glColor3f(1.0f, 1.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glVertex2f(gaugeX + 2, gaugeY + 2);
    glVertex2f(gaugeX + gaugeWidth - 2, gaugeY + 2);
    glVertex2f(gaugeX + gaugeWidth - 2, gaugeY + 2 + fillHeight);
    glVertex2f(gaugeX + 2, gaugeY + 2 + fillHeight);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText(gaugeX - 5, gaugeY + gaugeHeight + 10, "POWER");
}

void UI::DrawScoreboard3D() {
    Begin2D();

    float boardX = 140;
    float boardY = WINDOW_HEIGHT - 80;
    float cellWidth = 90;
    float cellHeight = 50;

    // 배경
    glColor4f(0.0f, 0.1f, 0.3f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(boardX - 10, boardY - cellHeight - 10);
    glVertex2f(boardX + cellWidth * 10 + 60, boardY - cellHeight - 10);
    glVertex2f(boardX + cellWidth * 10 + 60, boardY + 30);
    glVertex2f(boardX - 10, boardY + 30);
    glEnd();

    // 테두리
    glColor3f(0.3f, 0.5f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boardX - 10, boardY - cellHeight - 10);
    glVertex2f(boardX + cellWidth * 10 + 60, boardY - cellHeight - 10);
    glVertex2f(boardX + cellWidth * 10 + 60, boardY + 30);
    glVertex2f(boardX - 10, boardY + 30);
    glEnd();

    // 프레임 번호 (1~10)
    glColor3f(0.0f, 0.8f, 0.8f);
    for (int i = 0; i < 10; i++) {
        char num[3];
        sprintf(num, "%d", i + 1);
        DrawTextCentered(boardX + i * cellWidth + cellWidth / 2, boardY + 10, num);
    }

    // 각 프레임 점수
    for (int i = 0; i < 10; i++) {
        float x = boardX + i * cellWidth;

        // 셀 구분선
        glColor3f(0.3f, 0.5f, 0.8f);
        glBegin(GL_LINES);
        glVertex2f(x, boardY);
        glVertex2f(x, boardY - cellHeight);
        glEnd();

        // 투구 결과 표시
        glColor3f(1.0f, 1.0f, 1.0f);
        char throwStr[10] = "";

        if (frames[i].isComplete || i < currentFrame || (i == currentFrame && currentThrow == 2)) {
            if (frames[i].isStrike) {
                strcpy(throwStr, "S");
            }
            else if (frames[i].isSpare) {
                sprintf(throwStr, "%d /", frames[i].firstThrow);
            }
            else if (frames[i].isComplete) {
                sprintf(throwStr, "%d %d", frames[i].firstThrow, frames[i].secondThrow);
            }
            else if (i == currentFrame && currentThrow == 2) {
                sprintf(throwStr, "%d", frames[i].firstThrow);
            }
        }

        DrawTextCentered(x + cellWidth / 2, boardY - 20, throwStr);

        // 누적 점수
        if (frames[i].isComplete) {
            glColor3f(0.8f, 0.8f, 0.0f);
            char scoreStr[5];
            sprintf(scoreStr, "%d", frames[i].totalScore);
            DrawTextCentered(x + cellWidth / 2, boardY - 42, scoreStr);
        }
    }

    // 마지막 구분선
    glColor3f(0.3f, 0.5f, 0.8f);
    glBegin(GL_LINES);
    glVertex2f(boardX + 10 * cellWidth, boardY);
    glVertex2f(boardX + 10 * cellWidth, boardY - cellHeight);
    glEnd();

    // 총점 표시
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText(boardX + 10 * cellWidth + 10, boardY + 10, "TOTAL");

    int totalScore = 0;
    for (int i = 0; i < 10; i++) {
        if (frames[i].isComplete) totalScore = frames[i].totalScore;
    }
    char totalStr[5];
    sprintf(totalStr, "%d", totalScore);
    glColor3f(0.0f, 1.0f, 0.0f);
    DrawTextCentered(boardX + 10 * cellWidth + 35, boardY - 25, totalStr);

    End2D();
}

void UI::DrawSpinButtons() {
    for (int i = 0; i < 3; i++) {
        Button& btn = spinButtons[i];

        // 버튼 배경
        if (btn.isSelected) {
            glColor4f(0.2f, 0.6f, 0.2f, 0.9f);  // 선택됨 - 녹색
        }
        else if (btn.isHovered) {
            glColor4f(0.4f, 0.4f, 0.6f, 0.9f);  // 호버 - 밝은 색
        }
        else {
            glColor4f(0.2f, 0.2f, 0.3f, 0.9f);  // 기본 - 어두운 색
        }

        glBegin(GL_QUADS);
        glVertex2f(btn.x, btn.y);
        glVertex2f(btn.x + btn.width, btn.y);
        glVertex2f(btn.x + btn.width, btn.y + btn.height);
        glVertex2f(btn.x, btn.y + btn.height);
        glEnd();

        // 테두리
        if (btn.isSelected) {
            glColor3f(0.0f, 1.0f, 0.0f);
            glLineWidth(3.0f);
        }
        else {
            glColor3f(0.7f, 0.7f, 0.7f);
            glLineWidth(1.0f);
        }

        glBegin(GL_LINE_LOOP);
        glVertex2f(btn.x, btn.y);
        glVertex2f(btn.x + btn.width, btn.y);
        glVertex2f(btn.x + btn.width, btn.y + btn.height);
        glVertex2f(btn.x, btn.y + btn.height);
        glEnd();

        // 텍스트
        glColor3f(1.0f, 1.0f, 1.0f);
        DrawTextCentered(btn.x + btn.width / 2, btn.y + btn.height / 2 - 5, btn.label);
    }

    // 스핀 라벨
    glColor3f(0.8f, 0.8f, 0.8f);
    DrawText(spinButtons[0].x - 50, spinButtons[0].y + 12, "Spin:");
}

// [신규 구현] 텍스처 미리보기 박스 그리기
void UI::DrawTexturePreview(GLuint texID, float x, float y, float size) {
    if (texID == 0) return;

    // 텍스처 색상이 왜곡되지 않도록 흰색 베이스 설정
    glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);

    float halfSize = size / 2.0f;

    // 텍스처가 입혀진 사각형 그리기
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x - halfSize, y - halfSize); // 좌하
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + halfSize, y - halfSize); // 우하
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + halfSize, y + halfSize); // 우상
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x - halfSize, y + halfSize); // 좌상
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // 노란색 테두리 그리기
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - halfSize, y - halfSize);
    glVertex2f(x + halfSize, y - halfSize);
    glVertex2f(x + halfSize, y + halfSize);
    glVertex2f(x - halfSize, y + halfSize);
    glEnd();
}

void UI::DrawMenu() {
    // 1. 어두운 배경 오버레이
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT); glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    // 2. 메뉴 박스 (크기 확대 버전 유지)
    float menuWidth = 600;
    float menuHeight = 450;
    float menuX = (WINDOW_WIDTH - menuWidth) / 2;
    float menuY = (WINDOW_HEIGHT - menuHeight) / 2;

    // 메뉴 배경
    glColor4f(0.15f, 0.15f, 0.2f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(menuX, menuY); glVertex2f(menuX + menuWidth, menuY);
    glVertex2f(menuX + menuWidth, menuY + menuHeight); glVertex2f(menuX, menuY + menuHeight);
    glEnd();

    // 메뉴 테두리
    glColor3f(0.8f, 0.7f, 0.2f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(menuX, menuY); glVertex2f(menuX + menuWidth, menuY);
    glVertex2f(menuX + menuWidth, menuY + menuHeight); glVertex2f(menuX, menuY + menuHeight);
    glEnd();

    // 3. 카테고리별 데이터 준비
    const char* categoryTitle = "";
    const char* currentItemText = "";
    char buffer[50];
    int currentIdx = 0;
    int maxCount = 0;
    GLuint previewTexID = 0; // [중요] 보여줄 텍스처 ID

    Lane& lane = Game::Instance().lane;

    switch (menuCategory) {
    case 0: // 공 선택
        categoryTitle = "< SELECT BALL >";
        {
            const char* ballNames[] = { "  Red Ball", "  Blue Ball", "  Green Ball" };
            currentItemText = ballNames[previewBall];
            // 공은 별도 함수로 그립니다
        }
        break;
    case 1: // 레인
        categoryTitle = "< LANE TEXTURE >";
        currentIdx = previewLaneTex;
        maxCount = lane.laneTextures.size();
        if (maxCount > 0) previewTexID = lane.laneTextures[currentIdx];
        break;
    case 2: // 벽
        categoryTitle = "< WALL TEXTURE >";
        currentIdx = previewWallTex;
        maxCount = lane.wallTextures.size();
        if (maxCount > 0) previewTexID = lane.wallTextures[currentIdx];
        break;
    case 3: // 천장
        categoryTitle = "< CEILING TEXTURE >";
        currentIdx = previewCeilingTex;
        maxCount = lane.ceilingTextures.size();
        if (maxCount > 0) previewTexID = lane.ceilingTextures[currentIdx];
        break;
    case 4: // 바닥
        categoryTitle = "< FLOOR TEXTURE >";
        currentIdx = previewFloorTex;
        maxCount = lane.floorTextures.size();
        if (maxCount > 0) previewTexID = lane.floorTextures[currentIdx];
        break;
    }

    // 4. 타이틀 출력
    glColor3f(1.0f, 1.0f, 0.0f);
    DrawTextLarge(WINDOW_WIDTH / 2 - 100 , menuY + menuHeight - 50, categoryTitle);

    // 5. 중앙 미리보기 (공 그림 or 텍스처 이미지)
    float previewCenterX = WINDOW_WIDTH / 2;
    float previewCenterY = WINDOW_HEIGHT / 2 + 30; // 위치 미세 조정

    if (menuCategory == 0) {
        // [공] 기존 원 그리기 함수 호출
        DrawBallPreview();
    }
    else {
        // [텍스처] 이미지 박스 그리기 (크기 150x150)
        if (previewTexID != 0) {
            DrawTexturePreview(previewTexID, previewCenterX, previewCenterY, 150.0f);
        }
        else {
            // 텍스처가 없을 때 (빈 상자 or 텍스트)
            glColor3f(0.5f, 0.5f, 0.5f);
            DrawTextCentered(previewCenterX, previewCenterY, "No Image");
        }
    }

    // 6. 아이템 이름/번호 출력 (미리보기 그림 아래)
    if (menuCategory != 0) {
        if (maxCount > 0)
            sprintf(buffer, "Texture %d / %d", currentIdx + 1, maxCount);
        else
            sprintf(buffer, "No Texture Found");
        currentItemText = buffer;
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    // [위치 조정] 미리보기 그림 아래로 글씨 내리기 (120 -> 90)
    DrawTextLarge(WINDOW_WIDTH / 2 - 75 , menuY + 90, currentItemText);

    // 7. 좌우 화살표 (미리보기 중앙 높이에 맞춤)
    float arrowY = previewCenterY;
    DrawTextLarge(menuX + 50, arrowY, "<");
    DrawText(menuX + 40, arrowY - 30, "LEFT");

    DrawTextLarge(menuX + menuWidth - 65, arrowY, ">");
    DrawText(menuX + menuWidth - 80, arrowY - 30, "RIGHT");

    // 8. 조작 안내
    glColor3f(0.6f, 0.6f, 0.6f);
    DrawTextCentered(WINDOW_WIDTH / 2, menuY + 50, "TAB: Change Category");
    DrawTextCentered(WINDOW_WIDTH / 2, menuY + 30, "LEFT/RIGHT: Change Item");
    DrawTextCentered(WINDOW_WIDTH / 2, menuY + 10, "ENTER: Apply Selection");
    DrawTextCentered(WINDOW_WIDTH / 2, menuY - 30, "M: Close Menu");
}

void UI::DrawBallPreview() {
    // 메뉴 중앙에 공 그리기 (2D 원으로 표현)
    float centerX = WINDOW_WIDTH / 2;
    float centerY = WINDOW_HEIGHT / 2 + 20;
    float radius = 60.0f;

    // 공 색상
    vec3 colors[] = {
        vec3(0.8f, 0.1f, 0.1f),  // 빨강
        vec3(0.1f, 0.2f, 0.8f),  // 파랑
        vec3(0.1f, 0.7f, 0.2f)   // 초록
    };

    vec3 color = colors[previewBall];

    // 공 그리기 (원)
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(centerX, centerY);
    for (int i = 0; i <= 32; i++) {
        float angle = 2.0f * 3.14159f * i / 32;
        glVertex2f(centerX + cos(angle) * radius, centerY + sin(angle) * radius);
    }
    glEnd();

    // 공 하이라이트 (광택 효과)
    glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
    glBegin(GL_TRIANGLE_FAN);
    float hx = centerX - radius * 0.3f;
    float hy = centerY + radius * 0.3f;
    float hr = radius * 0.4f;
    glVertex2f(hx, hy);
    for (int i = 0; i <= 16; i++) {
        float angle = 2.0f * 3.14159f * i / 16;
        glVertex2f(hx + cos(angle) * hr, hy + sin(angle) * hr);
    }
    glEnd();

    // 공 테두리
    glColor3f(0.3f, 0.3f, 0.3f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 32; i++) {
        float angle = 2.0f * 3.14159f * i / 32;
        glVertex2f(centerX + cos(angle) * radius, centerY + sin(angle) * radius);
    }
    glEnd();
}

void UI::ToggleMenu() {
    menuOpen = !menuOpen;
    if (menuOpen) {
        // [변경] 메뉴 열 때 현재 게임 상태를 UI 변수로 가져옴 (동기화)
        previewBall = selectedBall;

        // Lane의 현재 텍스처 인덱스 가져오기
        Lane& lane = Game::Instance().lane;
        previewLaneTex = lane.currentLaneIndex;
        previewWallTex = lane.currentWallIndex;
        previewCeilingTex = lane.currentCeilingIndex;
        previewFloorTex = lane.currentFloorIndex;
    }
}

// [추가] 탭 키용 카테고리 전환
void UI::NextCategory() {
    if (!menuOpen) return;
    menuCategory++;
    if (menuCategory > 4) menuCategory = 0;
}

void UI::MenuLeft() {
    if (!menuOpen) return;
    Lane& lane = Game::Instance().lane;

    switch (menuCategory) {
    case 0: // Ball
        previewBall--;
        if (previewBall < 0) previewBall = 2;
        break;
    case 1: // Lane Texture
        if (lane.laneTextures.empty()) return;
        previewLaneTex--;
        if (previewLaneTex < 0) previewLaneTex = lane.laneTextures.size() - 1;
        break;
    case 2: // Wall Texture
        if (lane.wallTextures.empty()) return;
        previewWallTex--;
        if (previewWallTex < 0) previewWallTex = lane.wallTextures.size() - 1;
        break;
    case 3: // Ceiling Texture
        if (lane.ceilingTextures.empty()) return;
        previewCeilingTex--;
        if (previewCeilingTex < 0) previewCeilingTex = lane.ceilingTextures.size() - 1;
        break;
    case 4: // Floor Texture
        if (lane.floorTextures.empty()) return;
        previewFloorTex--;
        if (previewFloorTex < 0) previewFloorTex = lane.floorTextures.size() - 1;
        break;
    }
}

void UI::MenuRight() {
    if (!menuOpen) return;
    Lane& lane = Game::Instance().lane;

    switch (menuCategory) {
    case 0: // Ball
        previewBall++;
        if (previewBall > 2) previewBall = 0;
        break;
    case 1: // Lane Texture
        if (lane.laneTextures.empty()) return;
        previewLaneTex++;
        if (previewLaneTex >= lane.laneTextures.size()) previewLaneTex = 0;
        break;
    case 2: // Wall Texture
        if (lane.wallTextures.empty()) return;
        previewWallTex++;
        if (previewWallTex >= lane.wallTextures.size()) previewWallTex = 0;
        break;
    case 3: // Ceiling Texture
        if (lane.ceilingTextures.empty()) return;
        previewCeilingTex++;
        if (previewCeilingTex >= lane.ceilingTextures.size()) previewCeilingTex = 0;
        break;
    case 4: // Floor Texture
        if (lane.floorTextures.empty()) return;
        previewFloorTex++;
        if (previewFloorTex >= lane.floorTextures.size()) previewFloorTex = 0;
        break;
    }
}

// [이름 변경] 선택 확정
void UI::SelectItem() {
    if (!menuOpen) return;

    // 1. 공 선택 적용
    selectedBall = previewBall;
    Game::Instance().ball.SetBallType(selectedBall);

    // 2. 텍스처 선택 적용
    Game::Instance().lane.SetLaneTexture(previewLaneTex);
    Game::Instance().lane.SetWallTexture(previewWallTex);
    Game::Instance().lane.SetCeilingTexture(previewCeilingTex);
    Game::Instance().lane.SetFloorTexture(previewFloorTex);
}

void UI::OnMouseClick(int x, int y) {
    // y 좌표 뒤집기 (GLUT은 위에서 아래로, OpenGL은 아래서 위로)
    y = WINDOW_HEIGHT - y;

    // 스핀 버튼 클릭 체크
    for (int i = 0; i < 3; i++) {
        if (IsPointInButton(x, y, spinButtons[i])) {
            // 모든 버튼 선택 해제
            for (int j = 0; j < 3; j++) {
                spinButtons[j].isSelected = false;
            }
            // 클릭한 버튼 선택
            spinButtons[i].isSelected = true;
            selectedSpin = spinButtons[i].spinType;
            break;
        }
    }
}

void UI::OnMouseMove(int x, int y) {
    y = WINDOW_HEIGHT - y;

    // 호버 상태 업데이트
    for (int i = 0; i < 3; i++) {
        spinButtons[i].isHovered = IsPointInButton(x, y, spinButtons[i]);
    }
}

bool UI::IsPointInButton(int x, int y, Button& btn) {
    return x >= btn.x && x <= btn.x + btn.width &&
        y >= btn.y && y <= btn.y + btn.height;
}

void UI::DrawText(float x, float y, const char* text, void* font) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

void UI::DrawTextLarge(float x, float y, const char* text) {
    DrawText(x, y, text, GLUT_BITMAP_TIMES_ROMAN_24);
}

void UI::DrawTextCentered(float x, float y, const char* text, void* font) {
    // 텍스트 너비 대략 계산
    int len = strlen(text);
    float width = len * 10;  // 대략적인 글자 너비
    DrawText(x - width / 2, y, text, font);
}

void UI::DrawGameState(GameState state) {
    Begin2D();

    glColor3f(1.0f, 1.0f, 1.0f);
    const char* stateText = "";

    switch (state) {
    case GameState::AIMING:
        stateText = "WASD: Move | SPACE: Charge | M: Menu";
        break;
    case GameState::CHARGING:
        stateText = "Release SPACE to throw!";
        break;
    case GameState::ROLLING:
        stateText = "";
        break;
    case GameState::PIN_ACTION:
        stateText = "";
        break;
    case GameState::GAME_OVER:
        stateText = "GAME OVER! Press R to restart";
        break;
    default:
        break;
    }

    DrawTextCentered(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 30, stateText);

    End2D();
}

void UI::DrawStrikeSpare(bool isStrike, bool isSpare) {
    if (!isStrike && !isSpare) return;

    Begin2D();

    glColor3f(1.0f, 1.0f, 0.0f);
    if (isStrike) {
        DrawTextLarge(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2, "STRIKE!");
    }
    else if (isSpare) {
        DrawTextLarge(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2, "SPARE!");
    }

    End2D();
}

void UI::SetCameraPitch(float pitch) {
    cameraPitch = pitch;
}

