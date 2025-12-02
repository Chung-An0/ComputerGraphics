// [중요] 헤더 파일 순서: GLEW를 먼저, 그 다음 GLUT를 포함해야 합니다.
#include <GL/glew.h>
#include <GL/freeglut.h> 
#include <iostream>

// 윈도우 크기
int windowWidth = 800;
int windowHeight = 600;

// --- 초기화 함수 ---
// [참고 자료: 7. culling.pdf, 8. light.pdf]
void init() {
    // 배경색: 어두운 회색 (흰색 물체가 잘 보이게)
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);


    glEnable(GL_DEPTH_TEST);


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    glEnable(GL_LIGHT0);     // 0번 광원 켜기

    // 광원 위치 및 색상 설정 (흰색 빛)
    GLfloat lightPos[] = { 0.0f, 10.0f, 5.0f, 1.0f };
    GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f }; // 은은한 환경광

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, whiteLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);


    GLfloat matWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matShininess[] = { 50.0f }; // 반짝임 정도

    // 앞면(Front)에 대해 재질 적용
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matWhite);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matWhite);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);


    glShadeModel(GL_SMOOTH);
}

// --- 창 크기 변경 콜백 ---
// [참고 자료: 5. projection.pdf]
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    if (h == 0) h = 1;


    glViewport(0, 0, w, h);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    // 시야각 45도, 종횡비, 가까운 면 1.0, 먼 면 100.0
    gluPerspective(45.0, (double)w / h, 1.0, 100.0);

    // 다시 모델뷰 행렬로 복귀
    glMatrixMode(GL_MODELVIEW);
}

// --- 핀 하나를 그리는 함수 ---
// [참고 자료: 6. transformation.pdf - 계층 구조 모델링]
void drawPinAt(float x, float z) {
    glPushMatrix();

    // 1. 위치 이동 (Translate)
    glTranslatef(x, 0.3f, z);


    glPushMatrix();
    glScalef(0.3f, 1.5f, 0.3f);
    glutSolidSphere(0.3, 20, 20); // 핀 몸통
    glPopMatrix();

    // 3. 핀 머리 부분 (작은 구 추가)
    glTranslatef(0.0f, 0.5f, 0.0f);
    glutSolidSphere(0.15, 20, 20);

    glPopMatrix();
}

// --- 핀 10개를 삼각형으로 배치 ---
void drawPins() {
    float startZ = -15.0f; // 핀이 놓일 기준 Z 위치 (멀리)
    float spacingX = 0.6f; // 가로 간격
    float spacingZ = 0.5f; // 깊이 간격

    // 4줄 삼각형 배치 (1 + 2 + 3 + 4 = 10개)
    drawPinAt(0.0f, startZ); // 1열

    drawPinAt(-spacingX / 2.0f, startZ - spacingZ); // 2열
    drawPinAt(spacingX / 2.0f, startZ - spacingZ);

    drawPinAt(-spacingX, startZ - spacingZ * 2.0f); // 3열
    drawPinAt(0.0f, startZ - spacingZ * 2.0f);
    drawPinAt(spacingX, startZ - spacingZ * 2.0f);

    drawPinAt(-spacingX * 1.5f, startZ - spacingZ * 3.0f); // 4열
    drawPinAt(-spacingX * 0.5f, startZ - spacingZ * 3.0f);
    drawPinAt(spacingX * 0.5f, startZ - spacingZ * 3.0f);
    drawPinAt(spacingX * 1.5f, startZ - spacingZ * 3.0f);
}

// --- 메인 디스플레이 콜백 ---
void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // [참고 자료: 6. transformation.pdf - 뷰 변환]

// eye(0, 3, 5): 위쪽에서 아래로 내려다봄
// at(0, 0, -10): 레인 끝쪽을 바라봄
// up(0, 1, 0): Y축이 위쪽
    gluLookAt(0.0, 3.0, 5.0,
        0.0, 0.0, -10.0,
        0.0, 1.0, 0.0);

    // === 1. 볼링 레인 (바닥) ===
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, -8.0f); // 바닥면으로 내리고 뒤로 밈
    glScalef(3.0f, 0.1f, 25.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // === 2. 볼링공 ===
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);   // 원점(레인 시작점)에 위치
    glutSolidSphere(0.3, 40, 40);
    glPopMatrix();

    // === 3. 볼링핀 10개 ===
    drawPins();

    glutSwapBuffers();
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);


    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Bowling Simulation - White Model");

    // GLEW 초기화 (최신 OpenGL 기능 사용 시 필요, 여기선 기본 기능만 쓰므로 선택사항이나 안전을 위해 추가)
    glewInit();

    init(); // 조명, 재질 등 초기화

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}