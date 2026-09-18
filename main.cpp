#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

#include <GL/glut.h>
#include <GL/glu.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include "Game.h"

static Game game;
static unsigned int lastTime = 0;

void display() {
    game.render();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    game.windowWidth = w;
    game.windowHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 0.5, 600.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    game.handleKeyDown(key, x, y);
}

void keyboardUp(unsigned char key, int x, int y) {
    game.handleKeyUp(key, x, y);
}

void special(int key, int x, int y) {
    game.handleSpecialKeyDown(key, x, y);
}

void specialUp(int key, int x, int y) {
    game.handleSpecialKeyUp(key, x, y);
}

void passiveMotion(int x, int y) {
    game.handlePassiveMouse(x, y);
}

void mouseMotion(int x, int y) {
    game.handlePassiveMouse(x, y);
}

void mouseClick(int button, int state, int x, int y) {
    game.handleMouseClick(button, state, x, y);
}

void timer(int value) {
    #ifdef _WIN32
    unsigned int currentTime = timeGetTime();
    #else
    unsigned int currentTime = (unsigned int)(clock() * 1000 / CLOCKS_PER_SEC);
    #endif

    if (lastTime == 0) lastTime = currentTime;
    float dt = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    // Clamp dt to avoid physics spiral on lag
    if (dt > 0.05f) dt = 0.05f;
    if (dt <= 0.0f) dt = 0.016f;

    game.update(dt);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // 60 FPS target
}

void initGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    // Deep cosmic space background color
    glClearColor(0.015f, 0.018f, 0.035f, 1.0f);

    // Default global ambient light
    GLfloat globalAmbient[] = { 0.18f, 0.18f, 0.22f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("3D Space Defender: OpenGL FreeGLUT Shooting Game");

    initGL();
    game.init(1280, 720);

    // Register Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutPassiveMotionFunc(passiveMotion);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutTimerFunc(16, timer, 0);

    std::cout << "========================================================\n";
    std::cout << "    3D Space Defender: OpenGL FreeGLUT Shooting Game    \n";
    std::cout << "========================================================\n";
    std::cout << " Controls:\n";
    std::cout << "   [MOUSE]      : Target Aiming / Crosshair\n";
    std::cout << "   [LEFT CLICK] : Fire Plasma Blasters (or SPACEBAR)\n";
    std::cout << "   [W, S]       : Up / Down\n";
    std::cout << "   [A, D]       : Left / Right\n";
    std::cout << "   [Q, E]       : Reverse / Forward Thrust\n";
    std::cout << "   [C]          : Switch Camera (3rd Person / Cockpit / Free)\n";
    std::cout << "   [P]          : Pause Game\n";
    std::cout << "   [R]          : Restart Game\n";
    std::cout << "   [ESC]        : Exit Game\n";
    std::cout << "========================================================\n";

    glutMainLoop();
    return 0;
}
