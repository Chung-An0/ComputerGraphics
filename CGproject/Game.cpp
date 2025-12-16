#include "Game.h"

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

    // 볼링핀 텍스처 로드
    Pin::InitTexture("texture/pin_texture.bmp");  // 파일 경로
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
        // (이제는 거의 안 머무름) 그래도 혹시 남아있으면 대기
        break;
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

    // ✅ ball.velocity를 참조로 전달 (공도 영향 받음)
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
        // ✅ 10프레임까지 끝나면 멈추지 말고 즉시 초기화
        if (ui.currentFrame >= 10) {
            ResetGame();                 // 초기 상태로 복귀
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
    }
}

void Game::OnMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        ui.OnMouseClick(x, y);
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

void MouseCallback(int button, int state, int x, int y) {
    Game::Instance().OnMouse(button, state, x, y);
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