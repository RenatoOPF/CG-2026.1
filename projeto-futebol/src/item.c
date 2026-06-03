#include "item.h"
#include "globals.h"
#include <GL/glut.h>
#include <math.h>

float item_x = 0.0f;
float item_z = 0.0f;
int item_ativo = 1;
float angulo_rotacao = 0.0f;

void inicializar_item() {
    item_x = 3.0f;
    item_z = 3.0f;
    item_ativo = 1;
}

void desenhar_item() {
    if (!item_ativo) return;

    angulo_rotacao += 2.0f;
    
    glPushMatrix();
        glTranslatef(item_x, 0.5f, item_z);
        glRotatef(angulo_rotacao, 0.0f, 1.0f, 0.0f);
        glColor3f(1.0f, 0.5f, 0.0f); // Laranja
        glutSolidOctahedron();
    glPopMatrix();
}

void verificar_colisao_item() {
    if (!item_ativo) return;
    
    float dx = bola_x - item_x;
    float dz = bola_z - item_z;
    float dist = sqrt(dx*dx + dz*dz);
    
    if (dist < 1.0f) {
        bola_vx *= 2.5f; // Boost de velocidade
        bola_vz *= 2.5f;
        item_ativo = 0;
    }
}