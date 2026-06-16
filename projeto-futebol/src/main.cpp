#include <GL/glut.h>
#include "globals.h"
#include "campo.h"
#include "bola.h"
#include "jogadores.h"
#include "placar.h"
#include "torcida.h"
#include "estadio.h"
#include "camera.h"
#include "som.h"
#include "texturas.h"
#include "iluminacao.h"

Ball                gBall;
std::vector<Player> gPlayers;
GameState           gGame;
CameraMode          gCamera = CAM_TV;

static bool keys[256]       = {};
static bool specialKeys[256] = {};
static int  tempoAnterior   = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    setupCamera();
    aplicarLuzes();
    desenharCeu();
    desenharSol();

    desenharEstadio();
    desenharTorcida();
    desenharCampo();

    // Sombras projetadas no gramado, antes dos objetos que as geram
    if (iniciarSombra()) {
        desenharJogadores(true);
        desenharSombraBola();
        desenharSombraGols();
        desenharSombraEstrutura();
        finalizarSombra();
    }

    desenharJogadores();
    desenharBola();
    desenharPlacar();

    glutSwapBuffers();
}

void timer(int) {
    int agora = glutGet(GLUT_ELAPSED_TIME);
    float dt  = (agora - tempoAnterior) / 1000.0f;
    tempoAnterior = agora;
    if (dt > 0.05f) dt = 0.05f;

    gGame.time += dt;

    // Apito inicial
    if (gGame.time > 0.2f && gGame.time < 0.4f && gGame.scoreA == 0 && gGame.scoreB == 0
        && !gGame.goalActive) {
        static bool apitou = false;
        if (!apitou) { playApito(); apitou = true; }
    }

    // Fim da partida: apito final uma única vez e congela o jogo
    if (gGame.time >= MATCH_DURATION && !gGame.matchOver) {
        gGame.matchOver = true;
        playApito();
    }

    // Câmera livre (drone): funciona em qualquer momento, inclusive após o fim
    if (gCamera == CAM_FREE) {
        float frente = 0, lado = 0, cima = 0, dyaw = 0, dpitch = 0;
        if (keys['w'] || keys['W']) frente += 1.0f;
        if (keys['s'] || keys['S']) frente -= 1.0f;
        if (keys['d'] || keys['D']) lado   += 1.0f;
        if (keys['a'] || keys['A']) lado   -= 1.0f;
        if (keys[' '])              cima   += 1.0f;   // Espaço: sobe
        if (keys['z'] || keys['Z']) cima   -= 1.0f;   // Z: desce
        if (specialKeys[GLUT_KEY_RIGHT]) dyaw   += 1.0f;
        if (specialKeys[GLUT_KEY_LEFT])  dyaw   -= 1.0f;
        if (specialKeys[GLUT_KEY_UP])    dpitch += 1.0f;
        if (specialKeys[GLUT_KEY_DOWN])  dpitch -= 1.0f;
        moverCameraLivre(dt, frente, lado, cima, dyaw, dpitch);
    }

    if (gGame.matchOver) {
        // Partida encerrada: bola e jogadores parados (resultado na tela)
    } else if (!gGame.goalActive) {
        float dirX = 0, dirZ = 0;
        if (gCamera != CAM_FREE) {
            // Na câmera TV o sinal inverte conforme o lado da câmera, para que
            // W/S/A/D sempre correspondam à direção visual na tela.
            float sign = (gCamera == CAM_TOP) ? 1.0f : gCamSign;
            if (keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP])    dirZ = -1.0f;
            if (keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN])  dirZ =  1.0f;
            if (keys['a'] || keys['A'] || specialKeys[GLUT_KEY_LEFT])  dirX = -1.0f;
            if (keys['d'] || keys['D'] || specialKeys[GLUT_KEY_RIGHT]) dirX =  1.0f;
            dirX *= sign;
            dirZ *= sign;
        }
        atualizarBola(dt, dirX, dirZ);
        atualizarIA(dt);
        verificarGol();
    } else {
        gGame.goalTimer -= dt;
        if (gGame.goalTimer <= 0.0f) {
            gGame.goalActive = false;
            gBall.x = gBall.z = 0.0f;
            gBall.y = BALL_R;
            gBall.vx = gBall.vz = 0.0f;
            initJogadores();
            playApito();
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int, int) {
    keys[key] = true;
    if (key == 'c' || key == 'C')
        gCamera = static_cast<CameraMode>((gCamera + 1) % 3);
    if (key == 'r' || key == 'R') {
        gBall = Ball{};
        gGame = GameState{};
        initJogadores();
        playApito();
    }
    if (key == 'f' || key == 'F') {
        gCamera = (gCamera == CAM_FREE) ? CAM_TV : CAM_FREE;
        if (gCamera == CAM_FREE) {
            glutSetCursor(GLUT_CURSOR_NONE);      // mouse-look: esconde o cursor
            glutWarpPointer(WIN_W / 2, WIN_H / 2);
        } else {
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
    if (key == 'l' || key == 'L') gLuzOn = !gLuzOn;
    if (key == 'h' || key == 'H') { gHorario = (gHorario + 1) % 3; atualizarCeu(); }
    // Atalho secreto (não documentado): adianta o relógio p/ faltarem 5 segundos
    if (key == 't' || key == 'T') {
        float alvo = MATCH_DURATION - 5.0f;
        gGame.time = (alvo > 0.0f) ? alvo : 0.0f;
    }
    if (key == 27) exit(0);
}
void keyboardUp(unsigned char key, int, int) { keys[key] = false; }
void specialDown(int key, int, int) { if (key < 256) specialKeys[key] = true; }
void specialUp(int key, int, int)   { if (key < 256) specialKeys[key] = false; }

// Mouse-look da câmera drone: usa o deslocamento em relação ao centro e
// reposiciona o cursor no centro, permitindo giro contínuo.
void mouseMotion(int x, int y) {
    if (gCamera != CAM_FREE) return;
    int cx = WIN_W / 2, cy = WIN_H / 2;
    int dx = x - cx, dy = y - cy;
    if (dx == 0 && dy == 0) return;     // evento gerado pelo próprio warp
    girarCameraLivre((float)dx, (float)dy);
    glutWarpPointer(cx, cy);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (double)w / h, 0.5, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Futebol 3D - CG 2026.1 | Estádio Rei Pelé");

    glEnable(GL_DEPTH_TEST);
    initTexturas();
    initIluminacao();
    atualizarCeu();

    initSom();
    initJogadores();
    tempoAnterior = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutPassiveMotionFunc(mouseMotion);   // mouse-look (sem botão)
    glutMotionFunc(mouseMotion);          // mouse-look (com botão)
    glutTimerFunc(16, timer, 0);

    printf("=== Futebol 3D ===\n");
    printf("WASD / Setas : mover bola\n");
    printf("C            : alternar camera (Bola / TV / Aerea)\n");
    printf("F            : camera livre (drone) - WASD voa, Espaco/Z sobe-desce, MOUSE olha\n");
    printf("L            : ligar/desligar iluminacao\n");
    printf("H            : horario (meio-dia / entardecer / noite)\n");
    printf("R            : reiniciar\n");
    printf("ESC          : sair\n");

    glutMainLoop();
    return 0;
}
