#include "placar.h"
#include "globals.h"
#include "bola.h"
#include "som.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

int placar_jogador = 0;
int placar_computador = 0;

void desenhar_texto(const char* texto, float x, float y) {
    glRasterPos2f(x, y);
    int len = strlen(texto);
    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto[i]);
    }
}

void desenhar_placar() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, LARGURA_JANELA, 0, ALTURA_JANELA);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(1.0f, 1.0f, 0.0f); // Texto amarelo
    
    char buffer[100];
    sprintf(buffer, "Jogador: %d  |  Computador: %d", placar_jogador, placar_computador);
    desenhar_texto(buffer, 20.0f, ALTURA_JANELA - 30.0f);
    
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void verificar_gol() {
    if (bola_z > 14.5f && bola_x >= -2.0f && bola_x <= 2.0f) {
        placar_computador++;
        tocar_som_gol();
        resetar_bola();
    }
    else if (bola_z < -14.5f && bola_x >= -2.0f && bola_x <= 2.0f) {
        placar_jogador++;
        tocar_som_gol();
        resetar_bola();
    }
}