#include "campo.h"
#include <GL/glut.h>
#include <math.h>

void desenhar_circulo(float raio, int segmentos) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segmentos; i++) {
        float theta = 2.0f * 3.1415926f * (float)i / (float)segmentos;
        float x = raio * cosf(theta);
        float z = raio * sinf(theta);
        glVertex3f(x, 0.01f, z);
    }
    glEnd();
}

void desenhar_campo() {
    // Chão verde escuro
    glColor3f(0.1f, 0.5f, 0.2f);
    glBegin(GL_QUADS);
        glVertex3f(-12.0f, 0.0f, -17.0f);
        glVertex3f(-12.0f, 0.0f,  17.0f);
        glVertex3f( 12.0f, 0.0f,  17.0f);
        glVertex3f( 12.0f, 0.0f, -17.0f);
    glEnd();

    // Grama (campo principal)
    glColor3f(0.15f, 0.6f, 0.25f);
    glBegin(GL_QUADS);
        glVertex3f(-10.0f, 0.005f, -15.0f);
        glVertex3f(-10.0f, 0.005f,  15.0f);
        glVertex3f( 10.0f, 0.005f,  15.0f);
        glVertex3f( 10.0f, 0.005f, -15.0f);
    glEnd();

    // Linhas brancas
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    // Contorno
    glBegin(GL_LINE_LOOP);
        glVertex3f(-9.5f, 0.01f, -14.5f);
        glVertex3f(-9.5f, 0.01f,  14.5f);
        glVertex3f( 9.5f, 0.01f,  14.5f);
        glVertex3f( 9.5f, 0.01f, -14.5f);
    glEnd();

    // Linha central
    glBegin(GL_LINES);
        glVertex3f(-9.5f, 0.01f, 0.0f);
        glVertex3f( 9.5f, 0.01f, 0.0f);
    glEnd();

    // Círculo central
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.0f);
        desenhar_circulo(2.0f, 32);
    glPopMatrix();

    // Área Norte
    glBegin(GL_LINE_STRIP);
        glVertex3f(-4.0f, 0.01f, -14.5f);
        glVertex3f(-4.0f, 0.01f, -11.0f);
        glVertex3f( 4.0f, 0.01f, -11.0f);
        glVertex3f( 4.0f, 0.01f, -14.5f);
    glEnd();

    // Área Sul
    glBegin(GL_LINE_STRIP);
        glVertex3f(-4.0f, 0.01f,  14.5f);
        glVertex3f(-4.0f, 0.01f,  11.0f);
        glVertex3f( 4.0f, 0.01f,  11.0f);
        glVertex3f( 4.0f, 0.01f,  14.5f);
    glEnd();
    
    // Gols
    glColor3f(0.8f, 0.8f, 0.8f);
    
    // Gol Norte (Computador)
    glPushMatrix();
        glTranslatef(0.0f, 1.0f, -14.75f);
        glScalef(4.0f, 2.0f, 0.5f);
        glutWireCube(1.0);
    glPopMatrix();

    // Gol Sul (Jogador)
    glPushMatrix();
        glTranslatef(0.0f, 1.0f, 14.75f);
        glScalef(4.0f, 2.0f, 0.5f);
        glutWireCube(1.0);
    glPopMatrix();
}