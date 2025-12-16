#include "Game.h"
#include "Texture.h"
// 텍스처 로딩을 위해 Ball, Pin 헤더를 포함한다
#include "Ball.h"
#include "Pin.h"

// =============================================================
// B-lite Picking (Screen -> World)
// =============================================================

struct _Vec4d { double x, y, z, w; };

static _Vec4d _MulMat4Vec4(const double m[16], const _Vec4d& v) {
    // OpenGL column-major
    _Vec4d r;
    r.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
    r.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
    r.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
    r.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
    return r;
}

static bool _InvertMat4(const double m[16], double invOut[16]) {
    double inv[16];

    inv[0] = m[5] * m[10] * m[15] -
        m[5] * m[11] * m[14] -
        m[9] * m[6] * m[15] +
        m[9] * m[7] * m[14] +
        m[13] * m[6] * m[11] -
        m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
        m[4] * m[11] * m[14] +
        m[8] * m[6] * m[15] -
        m[8] * m[7] * m[14] -
        m[12] * m[6] * m[11] +
        m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
        m[4] * m[11] * m[13] -
        m[8] * m[5] * m[15] +
        m[8] * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
        m[4] * m[10] * m[13] +
        m[8] * m[5] * m[14] -
        m[8] * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
        m[1] * m[11] * m[14] +
        m[9] * m[2] * m[15] -
        m[9] * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
        m[0] * m[11] * m[14] -
        m[8] * m[2] * m[15] +
        m[8] * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
        m[0] * m[11] * m[13] +
        m[8] * m[1] * m[15] -
        m[8] * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
        m[0] * m[10] * m[13] -
        m[8] * m[1] * m[14] +
        m[8] * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
        m[1] * m[7] * m[14] -
        m[5] * m[2] * m[15] +
        m[5] * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
        m[0] * m[7] * m[14] +
        m[4] * m[2] * m[15] -
        m[4] * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
        m[0] * m[7] * m[13] -
        m[4] * m[1] * m[15] +
        m[4] * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
        m[0] * m[6] * m[13] +
        m[4] * m[1] * m[14] -
        m[4] * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
        m[1] * m[7] * m[10] +
        m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] -
        m[9] * m[2] * m[7] +
        m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
        m[0] * m[7] * m[10] -
        m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] +
        m[8] * m[2] * m[7] -
        m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
        m[0] * m[7] * m[9] +
        m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -
        m[8] * m[1] * m[7] +
        m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
        m[0] * m[6] * m[9] -
        m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] +
        m[8] * m[1] * m[6] -
        m[8] * m[2] * m[5];

    double det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (fabs(det) < 1e-12) return false;

    det = 1.0 / det;
    for (int i = 0; i < 16; i++) invOut[i] = inv[i] * det;
    return true;
}

static bool _UnProjectManual(
    double winX, double winY, double winZ,
    const double model[16], const double proj[16], const int viewport[4],
    double& outX, double& outY, double& outZ
) {
    double x = (winX - viewport[0]) / (double)viewport[2] * 2.0 - 1.0;
    double y = (winY - viewport[1]) / (double)viewport[3] * 2.0 - 1.0;
    double z = winZ * 2.0 - 1.0;

    double pm[16];
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            pm[c * 4 + r] =
                proj[0 * 4 + r] * model[c * 4 + 0] +
                proj[1 * 4 + r] * model[c * 4 + 1] +
                proj[2 * 4 + r] * model[c * 4 + 2] +
                proj[3 * 4 + r] * model[c * 4 + 3];
        }
    }

    double invPM[16];
    if (!_InvertMat4(pm, invPM)) return false;

    _Vec4d in{ x, y, z, 1.0 };
    _Vec4d out = _MulMat4Vec4(invPM, in);
    if (fabs(out.w) < 1e-12) return false;

    outX = out.x / out.w;
    outY = out.y / out.w;
    outZ = out.z / out.w;
    return true;
}

static bool _PickLanePoint(
    int mouseX, int mouseY,
    double yPlane,
    Camera& camera,
    double& hitX, double& hitY, double& hitZ
) {
    int viewport[4];
    double proj[16], model[16];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    camera.Apply();
    glGetDoublev(GL_MODELVIEW_MATRIX, model);

    double winX = (double)mouseX;
    double winY = (double)(viewport[3] - mouseY);

    double nx, ny, nz;
    double fx, fy, fz;
    bool okN = _UnProjectManual(winX, winY, 0.0, model, proj, viewport, nx, ny, nz);
    bool okF = _UnProjectManual(winX, winY, 1.0, model, proj, viewport, fx, fy, fz);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if (!okN || !okF) return false;

    double ox = nx, oy = ny, oz = nz;
    double dx = fx - nx, dy = fy - ny, dz = fz - nz;
    if (fabs(dy) < 1e-9) return false;

    double t = (yPlane - oy) / dy;
    if (t < 0.0) return false;

    hitX = ox + t * dx;
    hitY = oy + t * dy;
    hitZ = oz + t * dz;
    return true;
}

static void _DrawAimGuideLine(float startX, float startZ, SpinType spin) {
    const float PI = 3.14159265f;

    float safeHalf = (LANE_WIDTH * 0.5f) - BALL_RADIUS - 0.02f;
    if (safeHalf < 0.05f) safeHalf = 0.05f;

    float amp = safeHalf * 0.85f;
    float sign = 0.0f;
    if (spin == SpinType::LEFT_HOOK) sign = +1.0f;
    if (spin == SpinType::RIGHT_HOOK) sign = -1.0f;
    float pathAmp = sign * amp;

    GLboolean lightWas = glIsEnabled(GL_LIGHTING);
    GLboolean texWas = glIsEnabled(GL_TEXTURE_2D);

    if (lightWas) glDisable(GL_LIGHTING);
    if (texWas)   glDisable(GL_TEXTURE_2D);

    glLineWidth(2.5f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x00FF);

    if (spin == SpinType::STRAIGHT) glColor3f(0.9f, 0.9f, 0.9f);
    else if (spin == SpinType::LEFT_HOOK) glColor3f(0.2f, 0.9f, 1.0f);
    else glColor3f(1.0f, 0.4f, 0.9f);

    glBegin(GL_LINE_STRIP);
    const int N = 44;
    float y = BALL_RADIUS + 0.004f;

    for (int i = 0; i <= N; i++) {
        float t = (float)i / (float)N;
        float x = startX + pathAmp * sinf(PI * t);
        float z = (1.0f - t) * startZ + t * PIN_START_Z;
        glVertex3f(x, y, z);
    }
    glEnd();

    glDisable(GL_LINE_STIPPLE);

    if (texWas)   glEnable(GL_TEXTURE_2D);
    if (lightWas) glEnable(GL_LIGHTING);
}

// =============================================================
// Game
// =============================================================
Game& Game::Instance() {
    static Game instance;
    return instance;
}

void Game::Init() {
    state = GameState::AIMING;
    deltaTime = 0.0f;
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    pinSettleTimer = 0.0f;
    transitionTimer = 0.0f;
    pinsKnockedThisThrow = 0;
    showStrike = false;
    showSpare = false;
    messageTimer = 0.0f;
    spacePressed = false;

    for (int i = 0; i < 256; i++) {
        keys[i] = false;
    }

    lane.Init();
    // 공과 핀에 사용할 텍스처 로딩 (textures/ball.jpg, ball1.jpg, ball2.jpg, textures/pin.jpg)
    Ball::LoadTextures();
    //Pin::LoadTexture();
    Pin::InitTexture("texture/pin_texture.bmp");
    pins.ResetAll();
    ball.Reset(0.0f);
    ui.Reset();
    camera.SetMode(CameraMode::FIRST_PERSON);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glEnable(GL_FOG);
    GLfloat fogColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 10.0f);
    glFogf(GL_FOG_END, 30.0f);

}

void Game::Update() {
    CalculateDeltaTime();

    if (messageTimer > 0) {
        messageTimer -= deltaTime;
        if (messageTimer <= 0) {
            showStrike = false;
            showSpare = false;
        }
    }

    switch (state) {
    case GameState::AIMING:
        UpdateAiming();
        break;
    case GameState::CHARGING:
        UpdateCharging();
        break;
    case GameState::ROLLING:
        UpdateRolling();
        break;
    case GameState::PIN_ACTION:
        UpdatePinAction();
        break;
    case GameState::FRAME_END:
        UpdateFrameEnd();
        break;
    case GameState::GAME_OVER:
        break;
    }

    // 카메라 모드별 시점 갱신
    // BALL_FOLLOW는 볼링공이 굴러가지 않을 때에도 추적하게 하여 회색 화면을 방지한다.
    if (camera.mode == CameraMode::BALL_FOLLOW) {
        vec3 dir = (length(ball.velocity) > 0.1f) ? normalize(ball.velocity) : vec3(0.0f, 0.0f, -1.0f);
        camera.UpdateBallFollow(ball.position, dir);
    }
    else if (camera.mode == CameraMode::TOP_VIEW) {
        camera.UpdateTopView(ball.position);
    }
    else if (camera.mode == CameraMode::SIDE_VIEW) {
        camera.UpdateSideView(ball.position);
    }

    glutPostRedisplay();
}

void Game::Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ui.SetCameraPitch(camera.pitch);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    camera.Apply();

    lane.Draw();
    pins.Draw();

    // 조준/파워 단계에서만 가이드 라인
    if (!ui.menuOpen && (state == GameState::AIMING || state == GameState::CHARGING)) {
        _DrawAimGuideLine(camera.playerX, ball.position.z, ui.selectedSpin);
    }

    ui.DrawScoreboard3D();

    if (state != GameState::PIN_ACTION && state != GameState::FRAME_END) {
        ball.Draw();
    }
    else if (ball.isRolling) {
        ball.Draw();
    }

    ui.Draw();
    ui.DrawGameState(state);
    ui.DrawStrikeSpare(showStrike, showSpare);

    glutSwapBuffers();
}

void Game::SetState(GameState newState) {
    state = newState;

    switch (newState) {
    case GameState::AIMING:
        camera.SetMode(CameraMode::FIRST_PERSON);
        break;
    case GameState::ROLLING:
        camera.SetMode(CameraMode::BALL_FOLLOW);
        break;
    default:
        break;
    }
}

void Game::UpdateAiming() {
    float moveSpeed = 1.0f * deltaTime;
    float lookSpeed = 30.0f * deltaTime;

    if (keys['w'] || keys['W']) camera.LookUp(lookSpeed);
    if (keys['s'] || keys['S']) camera.LookDown(lookSpeed);

    if (keys['a'] || keys['A']) {
        camera.MoveLeft(moveSpeed);
        ball.position.x = camera.playerX;
    }
    if (keys['d'] || keys['D']) {
        camera.MoveRight(moveSpeed);
        ball.position.x = camera.playerX;
    }
}

void Game::UpdateCharging() {
    ui.UpdateGauge(deltaTime);
}

void Game::UpdateRolling() {
    ball.Update(deltaTime);

    camera.UpdateBallFollow(ball.position,
        length(ball.velocity) > 0.1f ? normalize(ball.velocity) : vec3(0, 0, -1));

    pins.CheckBallCollision(ball.position, ball.radius, ball.velocity, ball.angularVelocity);

    if (ball.position.z < PIN_START_Z - 2.0f || ball.IsStopped() || ball.isInGutter) {
        SetState(GameState::PIN_ACTION);
        pinSettleTimer = 2.0f;
    }
}

void Game::UpdatePinAction() {
    pins.Update(deltaTime);

    if (ball.isRolling) {
        ball.Update(deltaTime);
    }

    pinSettleTimer -= deltaTime;

    if (pinSettleTimer <= 0 || pins.AllSettled()) {
        int standingNow = pins.CountStanding();
        int knockedDown = 0;

        if (ui.currentThrow == 1) {
            knockedDown = 10 - standingNow;
        }
        else {
            int prevStanding = 10 - ui.frames[ui.currentFrame].firstThrow;
            knockedDown = prevStanding - standingNow;
        }

        pinsKnockedThisThrow = knockedDown;

        ui.RecordThrow(knockedDown);

        if (ui.currentThrow == 1 && knockedDown == 10) {
            showStrike = true;
            messageTimer = 2.0f;
        }
        else if (ui.currentThrow == 1 && ui.currentFrame > 0) {
            if (ui.frames[ui.currentFrame - 1].isSpare) {
                showSpare = true;
                messageTimer = 2.0f;
            }
        }

        SetState(GameState::FRAME_END);
        transitionTimer = 1.5f;
    }
}

void Game::UpdateFrameEnd() {
    transitionTimer -= deltaTime;

    if (transitionTimer <= 0) {
        //  10프레임까지 끝나면 멈추지 말고 즉시 초기화
        if (ui.currentFrame >= 10) {
            ResetGame();
            return;
        }
        NextThrow();
    }
}

void Game::OnKeyDown(unsigned char key) {
    keys[key] = true;

    switch (key) {
    case ' ':
        if (state == GameState::AIMING && !spacePressed) {
            spacePressed = true;
            ui.StartCharging();
            SetState(GameState::CHARGING);
        }
        break;

    case 'm':
    case 'M':
        if (state == GameState::AIMING) ui.ToggleMenu();
        break;

    case '1':
        ui.selectedBall = 0;
        ui.previewBall = 0;
        ball.SetBallType(0);
        break;
    case '2':
        ui.selectedBall = 1;
        ui.previewBall = 1;
        ball.SetBallType(1);
        break;
    case '3':
        ui.selectedBall = 2;
        ui.previewBall = 2;
        ball.SetBallType(2);
        break;

    case 'r':
    case 'R':
        if (state == GameState::GAME_OVER) {
            ResetGame();
        }
        break;

    case 13: // Enter
        if (ui.menuOpen) {
            ui.SelectBall();
            ball.SetBallType(ui.selectedBall);
        }
        break;

    case 27:
        exit(0);
        break;

    case 'c':
    case 'C':
        // 카메라 시점 순환: FIRST_PERSON → BALL_FOLLOW → TOP_VIEW → SIDE_VIEW → FIRST_PERSON
        if (camera.mode == CameraMode::FIRST_PERSON) {
            camera.SetMode(CameraMode::BALL_FOLLOW);
        }
        else if (camera.mode == CameraMode::BALL_FOLLOW) {
            camera.SetMode(CameraMode::TOP_VIEW);
        }
        else if (camera.mode == CameraMode::TOP_VIEW) {
            camera.SetMode(CameraMode::SIDE_VIEW);
        }
        else {
            camera.SetMode(CameraMode::FIRST_PERSON);
        }
        break;
    }
}

void Game::OnMouse(int button, int st, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && st == GLUT_DOWN) {
        // UI click first
        ui.OnMouseClick(x, y);

        // If the click was on the UI spin buttons, don't treat it as lane picking
        int yFlip = WINDOW_HEIGHT - y;
        bool onSpinUI = false;
        for (int i = 0; i < 3; i++) {
            if (ui.IsPointInButton(x, yFlip, ui.spinButtons[i])) {
                onSpinUI = true;
                break;
            }
        }

        // Lane picking: click to set the aim X (same as A/D)
        if (!onSpinUI && this->state == GameState::AIMING && !ui.menuOpen) {
            double hx, hy, hz;
            if (_PickLanePoint(x, y, (double)BALL_RADIUS, camera, hx, hy, hz)) {
                float limit = LANE_WIDTH / 2.0f - BALL_RADIUS;
                float nx = (float)hx;
                if (nx < -limit) nx = -limit;
                if (nx > limit) nx = limit;

                camera.playerX = nx;
                ball.position.x = nx;
            }
        }
    }
}

void Game::OnMouseMove(int x, int y) {
    ui.OnMouseMove(x, y);
}

void Game::OnKeyUp(unsigned char key) {
    keys[key] = false;

    if (key == ' ' && state == GameState::CHARGING) {
        spacePressed = false;

        float power = ui.StopCharging();
        if (power < 0.1f) power = 0.1f;

        ball.Launch(power, ui.selectedSpin);
        SetState(GameState::ROLLING);
    }
}

void Game::OnSpecialKey(int key) {
    if (ui.menuOpen) {
        switch (key) {
        case GLUT_KEY_LEFT:
            ui.MenuLeft();
            break;
        case GLUT_KEY_RIGHT:
            ui.MenuRight();
            break;
        }
    }
}

void Game::NextThrow() {
    if (ui.currentThrow == 1) {
        pins.ResetAll();
    }

    ResetForThrow();
    SetState(GameState::AIMING);
}

void Game::ResetForThrow() {
    ball.Reset(camera.playerX);
    camera.SetMode(CameraMode::FIRST_PERSON);
    camera.pitch = -5.0f;
}

// NextFrame(): 현재 버전에서는 사용하지 않습니다. 인터페이스 충돌을 피하기 위해 빈 구현을 제공합니다.
void Game::NextFrame() {
    // 이 함수는 사용되지 않습니다. 만약 프레임 진입 처리 로직이 필요하다면 여기서 구현하세요.
}

void Game::ResetGame() {
    ui.Reset();
    pins.ResetAll();
    ball.Reset(0.0f);
    camera.playerX = 0.0f;
    camera.SetMode(CameraMode::FIRST_PERSON);
    showStrike = false;
    showSpare = false;
    messageTimer = 0.0f;
    spacePressed = false;
    SetState(GameState::AIMING);
}

void Game::CalculateDeltaTime() {
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    if (deltaTime > 0.1f) deltaTime = 0.1f;
}

// ============ 콜백 함수들 ============

void DisplayCallback() {
    Game::Instance().Render();
}

void ReshapeCallback(int w, int h) {
    glViewport(0, 0, w, h);
}

void KeyboardCallback(unsigned char key, int x, int y) {
    Game::Instance().OnKeyDown(key);
}

void KeyboardUpCallback(unsigned char key, int x, int y) {
    Game::Instance().OnKeyUp(key);
}

void SpecialCallback(int key, int x, int y) {
    Game::Instance().OnSpecialKey(key);
}

void MouseCallback(int button, int st, int x, int y) {
    Game::Instance().OnMouse(button, st, x, y);
}

void MotionCallback(int x, int y) {
    Game::Instance().OnMouseMove(x, y);
}

void PassiveMotionCallback(int x, int y) {
    Game::Instance().OnMouseMove(x, y);
}

void TimerCallback(int value) {
    Game::Instance().Update();
    glutTimerFunc(16, TimerCallback, 0);
}