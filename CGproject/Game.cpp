#include "Game.h"

Game& Game::Instance() {
    static Game instance;
    return instance;
}

void Game::Init() {
    // 상태 초기화
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
    
    // 오브젝트 초기화
    lane.Init();
    pins.ResetAll();
    ball.Reset(0.0f);
    ui.Reset();
    camera.SetMode(CameraMode::FIRST_PERSON);
    
    // OpenGL 설정
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    glShadeModel(GL_SMOOTH);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    
    // 안개 효과 (분위기용)
    glEnable(GL_FOG);
    GLfloat fogColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 10.0f);
    glFogf(GL_FOG_END, 30.0f);
}

void Game::Update() {
    CalculateDeltaTime();
    
    // 메시지 타이머 업데이트
    if (messageTimer > 0) {
        messageTimer -= deltaTime;
        if (messageTimer <= 0) {
            showStrike = false;
            showSpare = false;
        }
    }
    
    // 상태별 업데이트
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
            // 대기
            break;
    }
    
    glutPostRedisplay();
}

void Game::Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 카메라 pitch를 UI에 전달 (천장 점수판 가시성)
    ui.SetCameraPitch(camera.pitch);

    // 3D 뷰 설정
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // 카메라 적용
    camera.Apply();
    
    // 3D 오브젝트 렌더링
    lane.Draw();
    pins.Draw();
    
    // 천장 점수판 (3D)
    ui.DrawScoreboard3D();

    // 공은 굴러가는 중이거나 조준 중일 때만 표시
    if (state != GameState::PIN_ACTION && state != GameState::FRAME_END) {
        ball.Draw();
    }
    else if (ball.isRolling) {
        ball.Draw();
    }
    
    // UI 렌더링
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
    // 키 입력에 따른 이동
    float moveSpeed = 1.0f * deltaTime;
    float lookSpeed = 30.0f * deltaTime;
    
    if (keys['w'] || keys['W']) {
        camera.LookUp(lookSpeed);
    }
    if (keys['s'] || keys['S']) {
        camera.LookDown(lookSpeed);
    }
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
    // 공 물리 업데이트
    ball.Update(deltaTime);
    
    // 카메라가 공 따라가기
    camera.UpdateBallFollow(ball.position, 
        length(ball.velocity) > 0.1f ? normalize(ball.velocity) : vec3(0, 0, -1));
    
    // 핀 충돌 체크
    pins.CheckBallCollision(ball.position, ball.radius, ball.velocity, ball.angularVelocity);
    
    // 공이 핀 영역 지나거나 멈추면
    if (ball.position.z < PIN_START_Z - 2.0f || ball.IsStopped() || ball.isInGutter) {
        SetState(GameState::PIN_ACTION);
        pinSettleTimer = 2.0f;  // 핀이 안정될 때까지 2초 대기
    }
}

void Game::UpdatePinAction() {
    // 핀 물리 업데이트
    pins.Update(deltaTime);
    
    // 공도 계속 굴러가게
    if (ball.isRolling) {
        ball.Update(deltaTime);
    }
    
    pinSettleTimer -= deltaTime;
    
    // 핀이 다 안정되거나 타이머 종료
    if (pinSettleTimer <= 0 || pins.AllSettled()) {
        // 쓰러진 핀 수 계산
        int standingNow = pins.CountStanding();
        int knockedDown = 0;
        
        if (ui.currentThrow == 1) {
            knockedDown = 10 - standingNow;
        }
        else {
            // 2투구: 이전에 서있던 핀 - 현재 서있는 핀
            int prevStanding = 10 - ui.frames[ui.currentFrame].firstThrow;
            knockedDown = prevStanding - standingNow;
        }
        
        pinsKnockedThisThrow = knockedDown;
        
        // 점수 기록
        ui.RecordThrow(knockedDown);
        
        // 스트라이크/스페어 체크
        if (ui.currentThrow == 1 && knockedDown == 10) {
            // 이미 다음 프레임으로 넘어갔으므로 이전 프레임 확인
            showStrike = true;
            messageTimer = 2.0f;
        }
        else if (ui.currentThrow == 1 && ui.currentFrame > 0) {
            // 스페어 체크 (이전 프레임)
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
        // 게임 종료 체크
        if (ui.currentFrame >= 10) {
            SetState(GameState::GAME_OVER);
            return;
        }
        
        // 다음 투구 준비
        NextThrow();
    }
}

void Game::OnKeyDown(unsigned char key) {
    keys[key] = true;
    
    switch (key) {
        case ' ':  // SPACE: 파워 게이지
            if (state == GameState::AIMING && !spacePressed) {
                spacePressed = true;
                ui.StartCharging();
                SetState(GameState::CHARGING);
            }
            break;
            
        case 'm':
        case 'M':  // 메뉴 토글
            if (state == GameState::AIMING) {
                ui.ToggleMenu();
            }
            break;
            
        case '1':  // 공 선택 (메뉴 없이도 가능)
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
        case 'R':  // 리셋
            if (state == GameState::GAME_OVER) {
                ResetGame();
            }
            break;

        case 13:  // Enter키 - 공 선택 확정
            if (ui.menuOpen) {
                ui.SelectBall();
                ball.SetBallType(ui.selectedBall);
            }
            break;

        case 27:  // ESC: 종료
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
        
        // 공 발사!
        float power = ui.StopCharging();
        if (power < 0.1f) power = 0.1f;  // 최소 파워
        
        ball.Launch(power, ui.selectedSpin);
        SetState(GameState::ROLLING);
    }
}

void Game::OnSpecialKey(int key) {
    // 메뉴에서 방향키 처리
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
    // 스트라이크였으면 핀 리셋
    if (ui.currentThrow == 1) {
        // 새 프레임 시작
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
    SetState(GameState::AIMING);
}

void Game::CalculateDeltaTime() {
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    // 너무 큰 델타타임 방지
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
    glutTimerFunc(16, TimerCallback, 0);  // ~60 FPS
}
