#include "iluminacao.h"
#include "globals.h"
#include "texturas.h"
#include <GL/glut.h>

bool gLuzOn = true;
bool gNoite = false;   // false = meio-dia, true = entardecer (sol baixo)

// Direção do campo para o sol. Única fonte de luz do projeto: a mesma direção
// alimenta a luz direcional, a projeção de sombra e o disco solar visível, para
// que iluminação, sombras e o sol no céu fiquem coerentes. De dia o sol é mais
// alto; ao entardecer fica baixo no horizonte (sombras longas e luz alaranjada).
static void sunDir(float out[3]) {
    if (!gNoite) { out[0] = -0.60f; out[1] = 0.48f; out[2] = -0.30f; }  // ~35°: visível e sombras médias
    else         { out[0] = -0.88f; out[1] = 0.20f; out[2] = -0.30f; }  // ~12°: entardecer, sombras longas
}

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
    const GLfloat pos[] = {d[0], d[1], d[2], 0.0f};   // w=0: luz direcional (sol distante)

    if (!gNoite) {
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
    } else {
        // Entardecer: sol baixo e alaranjado, ambiente quente
        const GLfloat amb[] = {0.26f, 0.21f, 0.20f, 1.0f};
        const GLfloat dif[] = {0.85f, 0.55f, 0.32f, 1.0f};
        const GLfloat spc[] = {0.40f, 0.28f, 0.18f, 1.0f};
        const GLfloat glb[] = {0.16f, 0.13f, 0.14f, 1.0f};
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

    glColor4f(0.0f, 0.0f, 0.0f, gNoite ? 0.45f : 0.50f);

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
    if (gNoite) glClearColor(0.92f, 0.55f, 0.32f, 1.0f);   // entardecer alaranjado
    else        glClearColor(0.52f, 0.74f, 0.95f, 1.0f);   // azul de meio-dia
}

// Esfera de céu texturizada (imagem panorâmica). Raio grande o suficiente para
// envolver a cena; desenhada sem luz e sem escrever profundidade, então tudo
// aparece na frente dela.
void desenharCeu() {
    GLuint tex = gNoite ? gTexCeuTarde : gTexCeu;
    if (tex == 0) tex = gTexCeu;   // sem céu de entardecer: usa o de dia
    if (tex == 0) return;          // sem imagem alguma: fica só o glClear

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

    float cr, cg, cb;
    if (!gNoite) { cr = 1.0f; cg = 0.97f; cb = 0.84f; }   // meio-dia
    else         { cr = 1.0f; cg = 0.60f; cb = 0.28f; }   // entardecer

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);     // aditivo: brilho

    glPushMatrix();
    glTranslatef(sx, sy, sz);
    glColor4f(cr, cg, cb, 0.16f); glutSolidSphere(26.0f, 20, 20);   // halo externo
    glColor4f(cr, cg, cb, 0.32f); glutSolidSphere(16.0f, 20, 20);   // halo interno
    glColor4f(cr, cg, cb, 1.0f);  glutSolidSphere(9.0f, 24, 24);    // núcleo
    glPopMatrix();

    glPopAttrib();
}
