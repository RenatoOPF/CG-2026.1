#include "iluminacao.h"
#include "globals.h"
#include "texturas.h"
#include <GL/glut.h>

bool gLuzOn   = true;
int  gHorario = HORA_DIA;   // meio-dia / entardecer / noite

// Direção do campo para o astro luminoso (sol de dia/tarde, lua à noite). A
// mesma direção alimenta a luz direcional, a projeção de sombra e o disco
// visível, mantendo iluminação, sombras e céu coerentes entre si.
static void sunDir(float out[3]) {
    switch (gHorario) {
        case HORA_TARDE: out[0] = -0.85f; out[1] = 0.38f; out[2] = -0.30f; break; // sol baixo (sombras médias-longas)
        case HORA_NOITE: out[0] = -0.35f; out[1] = 0.65f; out[2] = -0.25f; break; // lua alta
        default:         out[0] = -0.60f; out[1] = 0.48f; out[2] = -0.30f; break; // sol médio
    }
}

// Torres de refletores (4 cantos) — devem coincidir com estadio.cpp. Usadas só
// à noite, como luz artificial do estádio.
static const float TORRE_X = 38.8f;   // SXout + 2
static const float TORRE_Z = 50.0f;   // SZout + 2
static const float TORRE_Y = 27.5f;   // TH + 0.5 (plataforma de refletores)

void initIluminacao() {
    glEnable(GL_LIGHTING);

    // glColor define o material (ambiente + difuso): mantém todas as chamadas
    // glColor3f existentes no projeto funcionando como cor de superfície.
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // Especular discreto (sol não deixa tudo plástico)
    const GLfloat spec[] = {0.16f, 0.16f, 0.15f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 24.0f);

    glEnable(GL_NORMALIZE);                              // glScalef não-uniforme
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);     // faces vistas por trás

    // LIGHT1..LIGHT4: refletores das torres (spots) — parâmetros fixos, ligados
    // apenas no horário de noite (ver aplicarLuzes).
    const GLfloat spotDif[] = {0.55f, 0.53f, 0.47f, 1.0f};
    const GLfloat spotSpc[] = {0.35f, 0.34f, 0.30f, 1.0f};
    for (int i = 0; i < 4; i++) {
        GLenum L = GL_LIGHT1 + i;
        glLightfv(L, GL_DIFFUSE,  spotDif);
        glLightfv(L, GL_SPECULAR, spotSpc);
        glLightf(L, GL_SPOT_CUTOFF,   45.0f);
        glLightf(L, GL_SPOT_EXPONENT,  6.0f);
        glLightf(L, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(L, GL_LINEAR_ATTENUATION,   0.005f);
    }
}

void aplicarLuzes() {
    if (!gLuzOn) {
        glDisable(GL_LIGHTING);
        return;
    }
    glEnable(GL_LIGHTING);
    // Sem refletores artificiais: o sol (LIGHT0) é a única fonte.
    for (int i = 1; i < 8; i++) glDisable(GL_LIGHT0 + i);

    float d[3]; sunDir(d);
    const GLfloat pos[] = {d[0], d[1], d[2], 0.0f};   // w=0: luz direcional (distante)

    if (gHorario == HORA_TARDE) {
        // Entardecer: sol baixo e alaranjado, ambiente quente
        const GLfloat amb[] = {0.26f, 0.21f, 0.20f, 1.0f};
        const GLfloat dif[] = {0.85f, 0.55f, 0.32f, 1.0f};
        const GLfloat spc[] = {0.40f, 0.28f, 0.18f, 1.0f};
        const GLfloat glb[] = {0.16f, 0.13f, 0.14f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glb);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
    } else if (gHorario == HORA_NOITE) {
        // Noite: luar bem fraco e azulado + refletores artificiais das torres
        const GLfloat amb[] = {0.05f, 0.06f, 0.10f, 1.0f};
        const GLfloat dif[] = {0.12f, 0.14f, 0.24f, 1.0f};
        const GLfloat spc[] = {0.12f, 0.14f, 0.20f, 1.0f};
        const GLfloat glb[] = {0.04f, 0.05f, 0.08f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glb);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

        // 4 refletores nas torres apontando para o campo
        const float sgx[] = {-1, 1, -1, 1};
        const float sgz[] = {-1, -1, 1, 1};
        for (int i = 0; i < 4; i++) {
            GLenum L = GL_LIGHT1 + i;
            GLfloat p[]   = {sgx[i] * TORRE_X, TORRE_Y, sgz[i] * TORRE_Z, 1.0f};
            GLfloat dir[] = {sgx[i] * 6.0f - p[0], -TORRE_Y, sgz[i] * 10.0f - p[2]};
            glLightfv(L, GL_POSITION, p);
            glLightfv(L, GL_SPOT_DIRECTION, dir);
            glEnable(L);
        }
    } else {
        // Meio-dia: difusa + ambiente calibrados para NÃO saturar o gramado
        // claro (senão a faixa escura destoaria demais sob a luz).
        const GLfloat amb[] = {0.32f, 0.32f, 0.33f, 1.0f};
        const GLfloat dif[] = {0.82f, 0.80f, 0.74f, 1.0f};
        const GLfloat spc[] = {0.35f, 0.35f, 0.32f, 1.0f};
        const GLfloat glb[] = {0.18f, 0.18f, 0.20f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glb);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
    }
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_LIGHT0);
}

bool iniciarSombra() {
    if (!gLuzOn) return false;   // sem luz, sem sombra

    float d[3]; sunDir(d);
    float lx = d[0], ly = d[1], lz = d[2];

    // Matriz (column-major) que projeta cada vértice no plano y=0 na direção
    // da luz: x' = x - (lx/ly)y, z' = z - (lz/ly)y
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

    // Stencil: cada pixel de sombra é pintado uma única vez (evita dobra)
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    glColor4f(0.0f, 0.0f, 0.0f, (gHorario == HORA_NOITE) ? 0.30f : 0.50f);

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
    // Usado só como fundo quando não há imagem de céu carregada.
    switch (gHorario) {
        case HORA_TARDE: glClearColor(0.92f, 0.55f, 0.32f, 1.0f); break; // alaranjado
        case HORA_NOITE: glClearColor(0.04f, 0.05f, 0.12f, 1.0f); break; // azul-noite
        default:         glClearColor(0.52f, 0.74f, 0.95f, 1.0f); break; // azul-dia
    }
}

// Esfera de céu texturizada (imagem panorâmica). Raio grande o suficiente para
// envolver a cena; desenhada sem luz e sem escrever profundidade, então tudo
// aparece na frente dela.
void desenharCeu() {
    GLuint tex = 0;
    switch (gHorario) {
        case HORA_TARDE: tex = gTexCeuTarde ? gTexCeuTarde : gTexCeu; break;
        case HORA_NOITE: tex = gTexCeuNoite; break;   // sem imagem: usa fundo escuro
        default:         tex = gTexCeu; break;
    }
    if (tex == 0) return;          // sem imagem: fica só o glClear (cor do céu)

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1.0f, 1.0f, 1.0f);

    GLUquadric* q = gluNewQuadric();
    gluQuadricTexture(q, GL_TRUE);
    gluQuadricNormals(q, GLU_NONE);
    gluQuadricOrientation(q, GLU_INSIDE);   // vista de dentro

    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);    // eixo da esfera (z) -> vertical (y)
    gluSphere(q, 450.0, 28, 20);
    glPopMatrix();

    gluDeleteQuadric(q);
    glPopAttrib();
}

// Disco solar visível, na direção exata da luz. Núcleo brilhante + halos
// aditivos. Auto-iluminado (sem GL_LIGHTING). Mais perto que o céu (450) para
// aparecer na frente dele.
void desenharSol() {
    if (!gLuzOn) return;

    float d[3]; sunDir(d);
    float L = sqrtf(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    // Mais perto do estádio (parallax baixa a altura aparente) e menor.
    const float D = 150.0f;
    float sx = d[0]/L * D, sy = d[1]/L * D, sz = d[2]/L * D;

    float cr, cg, cb, haloA;
    switch (gHorario) {
        case HORA_TARDE: cr = 1.0f; cg = 0.60f; cb = 0.28f; haloA = 0.16f; break; // sol-poente
        case HORA_NOITE: cr = 0.86f; cg = 0.90f; cb = 1.0f; haloA = 0.08f; break; // lua pálida
        default:         cr = 1.0f; cg = 0.97f; cb = 0.84f; haloA = 0.16f; break; // sol
    }

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);     // aditivo: brilho

    glPushMatrix();
    glTranslatef(sx, sy, sz);
    glColor4f(cr, cg, cb, haloA);        glutSolidSphere(26.0f, 20, 20);   // halo externo
    glColor4f(cr, cg, cb, haloA * 2.0f); glutSolidSphere(16.0f, 20, 20);   // halo interno
    glColor4f(cr, cg, cb, 1.0f);         glutSolidSphere(9.0f, 24, 24);    // núcleo
    glPopMatrix();

    glPopAttrib();
}
