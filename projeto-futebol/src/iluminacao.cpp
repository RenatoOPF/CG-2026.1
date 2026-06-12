#include "iluminacao.h"
#include "globals.h"
#include <GL/glut.h>

bool gLuzOn = true;
bool gNoite = false;

// Posições das torres de iluminação — deve coincidir com estadio.cpp
static const float TORRE_X = 38.8f;   // SXout + 2
static const float TORRE_Z = 46.8f;   // SZout + 2
static const float TORRE_Y = 27.5f;   // TH + 0.5 (plataforma de refletores)

void initIluminacao() {
    glEnable(GL_LIGHTING);

    // glColor passa a definir o material (ambiente + difuso): permite manter
    // todas as chamadas glColor3f existentes no projeto
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // Brilho especular discreto para todos os objetos
    const GLfloat spec[] = {0.25f, 0.25f, 0.25f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);

    // glScalef não-uniforme (jogadores, torcida) exige renormalização
    glEnable(GL_NORMALIZE);
    // Faces vistas por trás (rede do gol, arquibancadas) recebem luz correta
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // LIGHT1..LIGHT4: refletores das torres (spots) — parâmetros fixos
    const GLfloat spotDif[] = {1.0f, 0.97f, 0.85f, 1.0f};
    const GLfloat spotSpc[] = {0.6f, 0.6f, 0.55f, 1.0f};
    for (int i = 0; i < 4; i++) {
        GLenum L = GL_LIGHT1 + i;
        glLightfv(L, GL_DIFFUSE,  spotDif);
        glLightfv(L, GL_SPECULAR, spotSpc);
        glLightf(L, GL_SPOT_CUTOFF,   40.0f);
        glLightf(L, GL_SPOT_EXPONENT,  4.0f);
        glLightf(L, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(L, GL_LINEAR_ATTENUATION,   0.004f);
    }
}

void aplicarLuzes() {
    if (!gLuzOn) {
        glDisable(GL_LIGHTING);
        return;
    }
    glEnable(GL_LIGHTING);

    if (!gNoite) {
        // --- Dia: sol direcional (w=0) com ambiente forte ---
        const GLfloat amb[]  = {0.30f, 0.30f, 0.30f, 1.0f};
        const GLfloat dif[]  = {0.85f, 0.83f, 0.78f, 1.0f};
        const GLfloat spc[]  = {0.50f, 0.50f, 0.45f, 1.0f};
        const GLfloat pos[]  = {0.4f, 1.0f, 0.3f, 0.0f};
        const GLfloat globalAmb[] = {0.22f, 0.22f, 0.24f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glEnable(GL_LIGHT0);
        for (int i = 0; i < 4; i++) glDisable(GL_LIGHT1 + i);
    } else {
        // --- Noite: lua fraca azulada + 4 refletores spot nas torres ---
        const GLfloat amb[]  = {0.02f, 0.02f, 0.04f, 1.0f};
        const GLfloat dif[]  = {0.10f, 0.12f, 0.20f, 1.0f};
        const GLfloat spc[]  = {0.05f, 0.05f, 0.08f, 1.0f};
        const GLfloat pos[]  = {-0.3f, 1.0f, -0.2f, 0.0f};
        const GLfloat globalAmb[] = {0.07f, 0.07f, 0.10f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glEnable(GL_LIGHT0);

        const float sx[] = {-1, 1, -1, 1};
        const float sz[] = {-1, -1, 1, 1};
        for (int i = 0; i < 4; i++) {
            GLenum L = GL_LIGHT1 + i;
            GLfloat pos[]  = {sx[i] * TORRE_X, TORRE_Y, sz[i] * TORRE_Z, 1.0f};
            // Cada torre aponta para o quadrante mais próximo do campo
            GLfloat alvoX = sx[i] * 6.0f, alvoZ = sz[i] * 10.0f;
            GLfloat dir[] = {alvoX - pos[0], -TORRE_Y, alvoZ - pos[2]};
            glLightfv(L, GL_POSITION, pos);
            glLightfv(L, GL_SPOT_DIRECTION, dir);
            glEnable(L);
        }
    }
}

bool iniciarSombra() {
    if (!gLuzOn) return false;   // sem luz, sem sombra

    // Direção da luz dominante: sol de dia; à noite os refletores vêm de
    // cima, então a projeção é quase vertical
    float lx, ly, lz;
    if (!gNoite) { lx = 0.4f;  ly = 1.0f; lz = 0.3f;  }
    else         { lx = 0.12f; ly = 1.0f; lz = 0.08f; }

    // Matriz (column-major) que projeta cada vértice no plano y=0
    // seguindo a direção da luz: x' = x - (lx/ly)y, z' = z - (lz/ly)y
    GLfloat m[16] = {
        1.0f,    0.0f, 0.0f,    0.0f,
        -lx/ly,  0.0f, -lz/ly,  0.0f,
        0.0f,    0.0f, 1.0f,    0.0f,
        0.0f,    0.0f, 0.0f,    1.0f
    };

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT |
                 GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    // Stencil: cada pixel de sombra é pintado uma única vez, evitando que
    // partes sobrepostas (pernas + tronco projetados) escureçam em dobro
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    glColor4f(0.0f, 0.0f, 0.0f, 0.40f);

    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.0f);   // levemente acima do gramado
    glMultMatrixf(m);
    return true;
}

void finalizarSombra() {
    glPopMatrix();
    glPopAttrib();
}

void atualizarCeu() {
    if (gNoite) glClearColor(0.04f, 0.05f, 0.10f, 1.0f);
    else        glClearColor(0.52f, 0.74f, 0.95f, 1.0f);
}
