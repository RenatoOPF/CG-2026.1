#include "campo.h"
#include "globals.h"
#include <cmath>

static void circulo(float cx, float cz, float raio, int seg, float y = 0.1f) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; i++) {
        float a = i * 2.0f * M_PI / seg;
        glVertex3f(cx + raio * cosf(a), y, cz + raio * sinf(a));
    }
    glEnd();
}

// Disco preenchido (para pontos de penalti redondos)
static void disco(float cx, float cz, float raio, float y = 0.1f) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(cx, y, cz);
    for (int i = 0; i <= 16; i++) {
        float a = i * 2.0f * M_PI / 16.0f;
        glVertex3f(cx + raio * cosf(a), y, cz + raio * sinf(a));
    }
    glEnd();
}

static void drawCylinder(GLUquadric* q,
                         float x1, float y1, float z1,
                         float x2, float y2, float z2, float r) {
    float dx = x2-x1, dy = y2-y1, dz = z2-z1;
    float len = sqrtf(dx*dx + dy*dy + dz*dz);
    if (len < 1e-5f) return;
    glPushMatrix();
    glTranslatef(x1, y1, z1);
    float nz = dz / len;
    if (fabsf(nz) > 0.9999f) {
        if (nz < 0.0f) glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
    } else {
        float angle = acosf(nz) * 180.0f / (float)M_PI;
        glRotatef(angle, -dy/len, dx/len, 0.0f);
    }
    gluCylinder(q, r, r, len, 12, 1);
    glPopMatrix();
}

static void arcoCanto(float cx, float cz, float aInicio) {
    const float r = 0.7f;
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 16; i++) {
        float a = aInicio + i * (M_PI / 2.0f) / 16.0f;
        glVertex3f(cx + r * cosf(a), 0.1f, cz + r * sinf(a));
    }
    glEnd();
}

// Arco de penalti: só a parte fora da área
static void arcoFora(float cx, float cz, float ar, float boundZ, bool fora_acima, float y = 0.1f) {
    bool desenhando = false;
    for (int i = 0; i <= 180; i++) {
        float a  = i * 2.0f * M_PI / 180.0f;
        float ax = cx + ar * cosf(a);
        float az = cz + ar * sinf(a);
        bool valido = fora_acima ? (az > boundZ) : (az < boundZ);
        if (valido) {
            if (!desenhando) { glBegin(GL_LINE_STRIP); desenhando = true; }
            glVertex3f(ax, y, az);
        } else {
            if (desenhando) { glEnd(); desenhando = false; }
        }
    }
    if (desenhando) glEnd();
}

void desenharCampo() {
    const float hw = FW / 2.0f;
    const float hl = FL / 2.0f;

    // Gramado
    glColor3f(0.13f, 0.54f, 0.13f);
    glBegin(GL_QUADS);
        glVertex3f(-hw, 0.0f, -hl);
        glVertex3f( hw, 0.0f, -hl);
        glVertex3f( hw, 0.0f,  hl);
        glVertex3f(-hw, 0.0f,  hl);
    glEnd();

    // Listras
    const int NLISTRAS = 8;
    const float lw = FW / NLISTRAS;
    glColor3f(0.11f, 0.48f, 0.11f);
    glBegin(GL_QUADS);
    for (int i = 0; i < NLISTRAS; i += 2) {
        float x0 = -hw + i * lw, x1 = x0 + lw;
        glVertex3f(x0, 0.01f, -hl); glVertex3f(x1, 0.01f, -hl);
        glVertex3f(x1, 0.01f,  hl); glVertex3f(x0, 0.01f,  hl);
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    // Contorno
    glBegin(GL_LINE_LOOP);
        glVertex3f(-hw, 0.1f, -hl); glVertex3f( hw, 0.1f, -hl);
        glVertex3f( hw, 0.1f,  hl); glVertex3f(-hw, 0.1f,  hl);
    glEnd();

    // Meio campo
    glBegin(GL_LINES);
        glVertex3f(-hw, 0.1f, 0.0f); glVertex3f(hw, 0.1f, 0.0f);
    glEnd();

    // Círculo e ponto central
    circulo(0, 0, 5.0f, 48);
    disco(0.0f, 0.0f, 0.15f);

    // Áreas (AG_W x AG_L)
    const float AG_W = 13.4f, AG_L = 11.0f;
    glBegin(GL_LINE_STRIP);
        glVertex3f(-AG_W, 0.1f, -hl);   glVertex3f(-AG_W, 0.1f, -hl+AG_L);
        glVertex3f( AG_W, 0.1f, -hl+AG_L); glVertex3f( AG_W, 0.1f, -hl);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex3f(-AG_W, 0.1f,  hl);   glVertex3f(-AG_W, 0.1f,  hl-AG_L);
        glVertex3f( AG_W, 0.1f,  hl-AG_L); glVertex3f( AG_W, 0.1f,  hl);
    glEnd();

    // Área pequena
    const float AP_W = 6.1f, AP_L = 3.7f;
    glBegin(GL_LINE_STRIP);
        glVertex3f(-AP_W, 0.1f, -hl);   glVertex3f(-AP_W, 0.1f, -hl+AP_L);
        glVertex3f( AP_W, 0.1f, -hl+AP_L); glVertex3f( AP_W, 0.1f, -hl);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex3f(-AP_W, 0.1f,  hl);   glVertex3f(-AP_W, 0.1f,  hl-AP_L);
        glVertex3f( AP_W, 0.1f,  hl-AP_L); glVertex3f( AP_W, 0.1f,  hl);
    glEnd();

    // Pontos de pênalti (redondos)
    const float PEN = 7.3f;
    disco(0.0f, -hl + PEN, 0.18f);
    disco(0.0f,  hl - PEN, 0.18f);

    // Meia lua: arco completo fora da área
    const float AR = 6.1f;
    arcoFora(0.0f, -hl + PEN, AR, -hl + AG_L, true);   // sul: fora = acima da linha da área
    arcoFora(0.0f,  hl - PEN, AR,  hl - AG_L, false);  // norte: fora = abaixo da linha da área

    // Arcos de canto
    arcoCanto(-hw, -hl, 0.0f);
    arcoCanto( hw, -hl, (float)M_PI / 2.0f);
    arcoCanto( hw,  hl, (float)M_PI);
    arcoCanto(-hw,  hl, 3.0f * (float)M_PI / 2.0f);

    // Gols com cilindros
    const float GW = GOAL_W / 2.0f;
    const float GH = GOAL_H;
    const float GD = GOAL_D;
    const float GR = 0.05f;

    glColor3f(1.0f, 1.0f, 1.0f);
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);

    auto drawGoal = [&](float gz, float gdir) {
        float gb = gz + gdir * GD;
        // Postes verticais frontais
        drawCylinder(q, -GW, 0.0f, gz, -GW, GH, gz, GR);
        drawCylinder(q,  GW, 0.0f, gz,  GW, GH, gz, GR);
        // Travessa horizontal frontal
        drawCylinder(q, -GW, GH, gz,  GW, GH, gz, GR);
        // Barras superiores de profundidade
        drawCylinder(q, -GW, GH, gz, -GW, GH, gb, GR);
        drawCylinder(q,  GW, GH, gz,  GW, GH, gb, GR);
        // Barras inferiores de profundidade
        drawCylinder(q, -GW, 0.0f, gz, -GW, 0.0f, gb, GR);
        drawCylinder(q,  GW, 0.0f, gz,  GW, 0.0f, gb, GR);
        // Postes verticais traseiros
        drawCylinder(q, -GW, 0.0f, gb, -GW, GH, gb, GR);
        drawCylinder(q,  GW, 0.0f, gb,  GW, GH, gb, GR);
        // Barras traseiras (topo e base)
        drawCylinder(q, -GW, GH,   gb,  GW, GH,   gb, GR);
        drawCylinder(q, -GW, 0.0f, gb,  GW, 0.0f, gb, GR);

        // Rede (mantida como linhas)
        glColor3f(0.85f, 0.85f, 0.85f);
        glLineWidth(1.0f);
        const int NGRID = 6;
        glBegin(GL_LINES);
        for (int i = 0; i <= NGRID; i++) {
            float t = (float)i / NGRID;
            float rx = -GW + t * GOAL_W;
            glVertex3f(rx, 0.0f, gb); glVertex3f(rx, GH, gb);
            float ry = t * GH;
            glVertex3f(-GW, ry, gb); glVertex3f(GW, ry, gb);
        }
        glEnd();
        glColor3f(1.0f, 1.0f, 1.0f);
    };

    drawGoal(-hl, -1.0f);
    drawGoal( hl,  1.0f);

    gluDeleteQuadric(q);
    glLineWidth(1.0f);
}
