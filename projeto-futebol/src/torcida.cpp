#include "torcida.h"
#include "globals.h"
#include <GL/glut.h>
#include <cmath>

// Deve coincidir com estadio.cpp
static const float PISTA_L = 3.8f;
static const float PISTA_E = 5.0f;
static const int   NDEG    = 10;
static const float DEG_W   = 1.3f;
static const float DEG_H   = 1.2f;
static const int   NDEG_E  = 7;
static const float DEG_WE  = 1.4f;
static const float DEG_HE  = 1.1f;

// Torcedor sentado (compacto para não sobressair no degrau)
static void torcedor(float x, float y, float z,
                     float r, float g, float b,
                     float ola) {
    const float H_BODY = 0.5f + ola * 0.4f;
    const float H_HEAD = 0.18f;

    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(x, y + H_BODY / 2.0f, z);
    glScalef(0.55f, H_BODY, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.88f, 0.72f, 0.58f);
    glPushMatrix();
    glTranslatef(x, y + H_BODY + H_HEAD, z);
    glutSolidSphere(H_HEAD, 4, 4);
    glPopMatrix();
}

void desenharTorcida() {
    const float hw  = FW / 2.0f;   // 20
    const float hl  = FL / 2.0f;   // 30
    const float t   = gGame.time;

    const float SX0  = hw + PISTA_L;          // 23.8
    const float SZ0  = hl + PISTA_E;          // 35.0
    const float SXout = SX0 + NDEG * DEG_W;   // 36.8
    const float z0   = -(hl + PISTA_E);       // -35
    const float z1   =   hl + PISTA_E;        //  35

    // Ola: onda senoidal percorre o estádio
    const float WAVE_SPEED = 2.5f;
    const float WAVE_LEN   = 50.0f;

    auto ola = [&](float pos) -> float {
        float s = 0.5f + 0.5f * sinf(pos / WAVE_LEN * 2.0f * (float)M_PI - t * WAVE_SPEED);
        return s;  // 0..1
    };

    // --- Bancadas laterais ---
    const int COLS_L = 24;
    for (int k = 0; k < NDEG; k++) {
        float xMid = SX0 + k * DEG_W + DEG_W * 0.5f;
        float yBase = (k + 1) * DEG_H;

        for (int col = 0; col < COLS_L; col++) {
            float bz = z0 + (col + 0.5f) * (z1 - z0) / COLS_L;
            float o  = ola(bz + k * 4.0f);

            // Cor alternada por col+row (mistura de azul e amarelo/vermelho)
            float r, g, b;
            int cor = (col + k) % 3;
            if      (cor == 0) { r=0.9f; g=0.1f; b=0.1f; }   // vermelho
            else if (cor == 1) { r=1.0f; g=1.0f; b=1.0f; }   // branco
            else               { r=0.9f; g=0.7f; b=0.0f; }   // amarelo

            torcedor(-xMid, yBase, bz, r, g, b, o);
            torcedor( xMid, yBase, bz, r, g, b, o);
        }
    }

    // --- Bancadas de fundo (atrás dos gols) ---
    const int COLS_E = 18;
    for (int k = 0; k < NDEG_E; k++) {
        float zMid = SZ0 + k * DEG_WE + DEG_WE * 0.5f;
        float yBase = (k + 1) * DEG_HE;

        for (int col = 0; col < COLS_E; col++) {
            float bx = -SXout + (col + 0.5f) * (2.0f * SXout / COLS_E);
            float o_s = ola(bx + k * 4.0f + 10.0f);
            float o_n = ola(bx + k * 4.0f + 60.0f);

            float r, g, b;
            int cor = (col + k) % 3;
            if      (cor == 0) { r=0.9f; g=0.1f; b=0.1f; }
            else if (cor == 1) { r=1.0f; g=1.0f; b=1.0f; }
            else               { r=0.2f; g=0.2f; b=0.8f; }   // azul

            torcedor(bx, yBase, -zMid, r, g, b, o_s);  // sul
            torcedor(bx, yBase,  zMid, r, g, b, o_n);  // norte
        }
    }
}
