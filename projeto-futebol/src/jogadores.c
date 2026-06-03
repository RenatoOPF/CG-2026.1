#include "jogadores.h"
#include "globals.h"
#include "som.h"
#include <GL/glut.h>
#include <math.h>

#define NUM_JOGADORES 3

typedef struct {
    float x;
    float z;
    float velocidade;
} Jogador;

Jogador jogadores_inimigos[NUM_JOGADORES];

void inicializar_jogadores() {
    jogadores_inimigos[0].x = -4.0f; jogadores_inimigos[0].z = -5.0f; jogadores_inimigos[0].velocidade = 3.0f;
    jogadores_inimigos[1].x =  4.0f; jogadores_inimigos[1].z = -5.0f; jogadores_inimigos[1].velocidade = 3.5f;
    jogadores_inimigos[2].x =  0.0f; jogadores_inimigos[2].z = -8.0f; jogadores_inimigos[2].velocidade = 2.5f;
}

void desenhar_jogadores() {
    glColor3f(1.0f, 0.0f, 0.0f); // Vermelho
    for (int i = 0; i < NUM_JOGADORES; i++) {
        glPushMatrix();
            glTranslatef(jogadores_inimigos[i].x, 0.5f, jogadores_inimigos[i].z);
            glutSolidCube(1.0);
        glPopMatrix();
    }
}

void atualizar_jogadores(float dt) {
    for (int i = 0; i < NUM_JOGADORES; i++) {
        float dx = bola_x - jogadores_inimigos[i].x;
        float dz = bola_z - jogadores_inimigos[i].z;
        float dist = sqrt(dx*dx + dz*dz);
        
        // IA persegue a bola
        if (dist > 0.1f) {
            float dir_x = dx / dist;
            float dir_z = dz / dist;
            
            jogadores_inimigos[i].x += dir_x * jogadores_inimigos[i].velocidade * dt;
            jogadores_inimigos[i].z += dir_z * jogadores_inimigos[i].velocidade * dt;
        }

        // Colisão com a bola
        if (dist < 1.0f) {
            bola_vx += (dx / dist) * 2.0f;
            bola_vz += (dz / dist) * 2.0f;
            tocar_som_chute();
        }
    }
}

void desenhar_torcida() {
    glColor3f(0.0f, 0.0f, 1.0f); // Torcida Azul
    for (int i = -12; i <= 12; i += 2) {
        glPushMatrix();
            glTranslatef((float)i, 1.5f, -16.5f);
            glutSolidCube(1.0);
        glPopMatrix();
        
        glPushMatrix();
            glTranslatef((float)i, 1.5f, 16.5f);
            glutSolidCube(1.0);
        glPopMatrix();
    }
}