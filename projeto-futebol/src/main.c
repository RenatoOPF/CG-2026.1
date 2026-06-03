#include <GL/glut.h>
#include "globals.h"
#include "campo.h"
#include "bola.h"
#include "placar.h"
#include "jogadores.h"
#include "item.h"
#include "som.h"
#include <stdio.h>

float bola_x = 0.0f;
float bola_z = 0.0f;
float bola_vx = 0.0f;
float bola_vz = 0.0f;

int tempo_anterior = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0.0, 20.0, 25.0,  
              0.0, 0.0, 0.0,    
              0.0, 1.0, 0.0);   

    desenhar_campo();
    desenhar_torcida();
    desenhar_jogadores();
    desenhar_item();
    desenhar_bola();
    desenhar_placar();

    glutSwapBuffers();
}

void timer(int value) {
    int tempo_atual = glutGet(GLUT_ELAPSED_TIME);
    float dt = (tempo_atual - tempo_anterior) / 1000.0f;
    tempo_anterior = tempo_atual;

    if (dt > 0.05f) dt = 0.05f;

    atualizar_bola(dt);
    atualizar_jogadores(dt);
    verificar_colisao_item();
    verificar_gol();

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float proporcao = (float)w / (float)h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, proporcao, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    float forca = 15.0f; 
    switch (key) {
        case 27:
            // exit(0);
            break;
        case 'w':
        case 'W':
            bola_vz -= forca;
            tocar_som_chute();
            break;
        case 's':
        case 'S':
            bola_vz += forca;
            tocar_som_chute();
            break;
        case 'a':
        case 'A':
            bola_vx -= forca;
            tocar_som_chute();
            break;
        case 'd':
        case 'D':
            bola_vx += forca;
            tocar_som_chute();
            break;
        case 'r':
        case 'R':
            resetar_bola();
            inicializar_jogadores();
            inicializar_item();
            break;
    }
}

void init() {
    glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    tempo_anterior = glutGet(GLUT_ELAPSED_TIME);
    
    inicializar_som();
    inicializar_jogadores();
    inicializar_item();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(LARGURA_JANELA, ALTURA_JANELA);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Projeto Futebol CG - AB1");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    printf("Controles: W, A, S, D para empurrar a bola.\nR para resetar o jogo.\n");
    
    glutMainLoop();
    
    return 0;
}