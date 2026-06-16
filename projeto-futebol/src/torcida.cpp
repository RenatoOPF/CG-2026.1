#include "torcida.h"
#include "globals.h"
#include <GL/glut.h>
#include <cmath>

// Deve coincidir com a tigela de arquibancada em estadio.cpp
static const float PISTA_L = 3.8f;
static const float PISTA_E = 5.0f;
static const int   NB  = 10;     // nº de degraus (fileiras)
static const float STD = 1.3f;   // profundidade de cada degrau
static const float STH = 1.2f;   // altura de cada degrau

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

// Paleta de cores por índice (camisas da torcida)
static void corTorcedor(int idx, float& r, float& g, float& b) {
    switch (idx % 3) {
        case 0:  r=0.9f; g=0.1f; b=0.1f; break;   // vermelho
        case 1:  r=1.0f; g=1.0f; b=1.0f; break;   // branco
        default: r=0.9f; g=0.7f; b=0.0f; break;   // amarelo
    }
}

void desenharTorcida() {
    const float hw  = FW / 2.0f;   // 20
    const float hl  = FL / 2.0f;   // 30
    const float t   = gGame.time;

    const float SX0 = hw + PISTA_L;   // 23.8
    const float SZ0 = hl + PISTA_E;   // 35.0

    // Ola: onda senoidal percorre o estádio
    const float WAVE_SPEED = 2.5f;
    const float WAVE_LEN   = 50.0f;
    auto ola = [&](float pos) -> float {
        return 0.5f + 0.5f * sinf(pos / WAVE_LEN * 2.0f * (float)M_PI - t * WAVE_SPEED);
    };

    // --- Bancadas laterais (fileiras ao longo de Z, nos dois lados em X) ---
    const int COLS_L = 24;
    for (int k = 0; k < NB; k++) {
        float xMid  = SX0 + k * STD + STD * 0.5f;
        float yBase = (k + 1) * STH;
        for (int col = 0; col < COLS_L; col++) {
            float bz = -SZ0 + (col + 0.5f) * (2.0f * SZ0) / COLS_L;
            float o  = ola(bz + k * 4.0f);
            float r, g, b; corTorcedor(col + k, r, g, b);
            torcedor(-xMid, yBase, bz, r, g, b, o);
            torcedor( xMid, yBase, bz, r, g, b, o);
        }
    }

    // --- Bancadas de fundo (faixa central |x|<SX0; cantos preenchem o resto) ---
    const int COLS_E = 24;
    for (int k = 0; k < NB; k++) {
        float zMid  = SZ0 + k * STD + STD * 0.5f;
        float yBase = (k + 1) * STH;
        for (int col = 0; col < COLS_E; col++) {
            float bx = -SX0 + (col + 0.5f) * (2.0f * SX0) / COLS_E;
            float o_s = ola(bx + k * 4.0f + 10.0f);
            float o_n = ola(bx + k * 4.0f + 60.0f);
            float r, g, b; corTorcedor(col + k, r, g, b);
            torcedor(bx, yBase, -zMid, r, g, b, o_s);   // sul
            torcedor(bx, yBase,  zMid, r, g, b, o_n);   // norte
        }
    }

    // --- Cantos arredondados (quartos de arco ligando laterais e fundos) ---
    const int COLS_C = 7;
    const float quad[4][2] = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };
    for (auto& q : quad) {
        float sx = q[0], sz = q[1];
        for (int k = 0; k < NB; k++) {
            float rm    = (k + 0.5f) * STD;        // raio médio do degrau
            float yBase = (k + 1) * STH;
            for (int col = 0; col < COLS_C; col++) {
                float a = (col + 0.5f) / COLS_C * (float)M_PI * 0.5f;
                float bx = sx * (SX0 + rm * cosf(a));
                float bz = sz * (SZ0 + rm * sinf(a));
                float o  = ola((sx*sz) * a * 20.0f + k * 4.0f + 30.0f);
                float r, g, b; corTorcedor(col + k, r, g, b);
                torcedor(bx, yBase, bz, r, g, b, o);
            }
        }
    }
}
