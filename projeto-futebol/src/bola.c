#include "bola.h"
#include "globals.h"
#include <GL/glut.h>

void desenhar_bola() {
    glPushMatrix();
        glTranslatef(bola_x, 0.5f, bola_z);
        glColor3f(1.0f, 1.0f, 1.0f); // Branca
        glutSolidSphere(0.5, 32, 32);
        
        // Desenha algumas linhas na bola para parecer que ela gira/tem textura procedural
        glColor3f(0.0f, 0.0f, 0.0f);
        glutWireSphere(0.51, 8, 8);
    glPopMatrix();
}

void atualizar_bola(float dt) {
    bola_x += bola_vx * dt;
    bola_z += bola_vz * dt;
    
    // Atrito
    bola_vx *= 0.98f;
    bola_vz *= 0.98f;
    
    // Para a bola se a velocidade for muito baixa
    if (bola_vx > -0.05f && bola_vx < 0.05f) bola_vx = 0.0f;
    if (bola_vz > -0.05f && bola_vz < 0.05f) bola_vz = 0.0f;
    
    // Colisão com as laterais do campo (-9.5 a 9.5)
    if (bola_x > 9.0f) { bola_x = 9.0f; bola_vx = -bola_vx * 0.8f; }
    if (bola_x < -9.0f) { bola_x = -9.0f; bola_vx = -bola_vx * 0.8f; }
    
    // Colisão com as linhas de fundo (Z: -14.5 a 14.5)
    // O gol fica entre X=-2 e X=2
    if (bola_z > 14.0f) {
        if (bola_x < -2.0f || bola_x > 2.0f) {
            bola_z = 14.0f;
            bola_vz = -bola_vz * 0.8f;
        }
    }
    
    if (bola_z < -14.0f) {
        if (bola_x < -2.0f || bola_x > 2.0f) {
            bola_z = -14.0f;
            bola_vz = -bola_vz * 0.8f;
        }
    }
}

void resetar_bola() {
    bola_x = 0.0f;
    bola_z = 0.0f;
    bola_vx = 0.0f;
    bola_vz = 0.0f;
}