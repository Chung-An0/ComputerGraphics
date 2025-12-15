#include "Game.h"

int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Bowling Game - OpenGL");

    // GLEW 초기화
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
        return -1;
    }

    //콘솔 메시지 추가 예정

    // 게임 초기화
    Game::Instance().Init();

    // 콜백 등록
    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutKeyboardFunc(KeyboardCallback);
    glutKeyboardUpFunc(KeyboardUpCallback);
    glutSpecialFunc(SpecialCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MotionCallback);
    glutPassiveMotionFunc(PassiveMotionCallback);
    glutTimerFunc(16, TimerCallback, 0);

    // 메인 루프
    glutMainLoop();

    return 0;
}
