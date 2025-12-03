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
    
    cout << "==============================" << endl;
    cout << "    BOWLING GAME - OpenGL     " << endl;
    cout << "==============================" << endl;
    cout << endl;
    cout << "Controls:" << endl;
    cout << "  W/S    - Look Up/Down" << endl;
    cout << "  A/D    - Move Left/Right" << endl;
    cout << "  SPACE  - Hold to charge, release to throw" << endl;
    cout << "  M      - Toggle Menu" << endl;
    cout << "  1/2/3  - Select Ball Color" << endl;
    cout << "  Q      - Left Hook Spin" << endl;
    cout << "  E      - Right Hook Spin" << endl;
    cout << "  R      - Straight / Restart Game" << endl;
    cout << "  ESC    - Exit" << endl;
    cout << endl;
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "==============================" << endl;
    
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
