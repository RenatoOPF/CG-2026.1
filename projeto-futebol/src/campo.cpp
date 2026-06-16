#include "campo.h"
#include "globals.h"
#include "texturas.h"
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

// Estrutura (traves) de um gol: apenas os cilindros do quadro, sem a rede.
// Reutilizada pelo desenho normal e pelo passe de sombra.
static void framaGol(GLUquadric* q, float gz, float gdir) {
    const float GW = GOAL_W / 2.0f, GH = GOAL_H, GD = GOAL_D, GR = 0.05f;
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

    // Gramado texturizado (mapeamento: 1 tile a cada 2.5u)
    const float TS = 2.5f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexGrama);
    glColor3f(1.0f, 1.0f, 1.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,        0.0f);        glVertex3f(-hw, 0.0f,  hl);
        glTexCoord2f(FW/TS,       0.0f);        glVertex3f( hw, 0.0f,  hl);
        glTexCoord2f(FW/TS,       FL/TS);       glVertex3f( hw, 0.0f, -hl);
        glTexCoord2f(0.0f,        FL/TS);       glVertex3f(-hw, 0.0f, -hl);
    glEnd();

    // Faixas horizontais de corte: bandas ao longo de Z (atravessam a largura).
    // Contraste suave (0.85 vs 1.0): visível com a luz, sem ficar escuro demais.
    const int NLISTRAS = 12;
    const float lz = FL / NLISTRAS;
    glColor3f(0.85f, 0.85f, 0.85f);
    glBegin(GL_QUADS);
    for (int i = 0; i < NLISTRAS; i += 2) {
        float za = -hl + i * lz, zb = za + lz;
        float v0 = (hl - za) / TS, v1 = (hl - zb) / TS;
        glTexCoord2f(0.0f,  v0); glVertex3f(-hw, 0.01f, za);
        glTexCoord2f(FW/TS, v0); glVertex3f( hw, 0.01f, za);
        glTexCoord2f(FW/TS, v1); glVertex3f( hw, 0.01f, zb);
        glTexCoord2f(0.0f,  v1); glVertex3f(-hw, 0.01f, zb);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Linhas do campo: brancas puras, sem iluminação nem textura
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);

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

    glPopAttrib();   // restaura iluminação/textura para os gols

    // Gols com cilindros
    const float GW = GOAL_W / 2.0f;
    const float GH = GOAL_H;
    const float GD = GOAL_D;

    glColor3f(1.0f, 1.0f, 1.0f);
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);

    auto drawGoal = [&](float gz, float gdir) {
        float gb = gz + gdir * GD;
        framaGol(q, gz, gdir);   // estrutura (traves)

        // --- Rede: quads texturizados com furos (alpha test) cobrindo as
        //     faces traseira, laterais e superior do gol ---
        const float S = 3.0f;   // células da rede por unidade
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gTexRede);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.5f);
        glColor3f(0.92f, 0.92f, 0.92f);

        glBegin(GL_QUADS);
        // Face traseira (normal coerente com o winding; TWO_SIDE corrige o outro lado)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f,      0.0f); glVertex3f(-GW, 0.0f, gb);
        glTexCoord2f(GOAL_W*S,  0.0f); glVertex3f( GW, 0.0f, gb);
        glTexCoord2f(GOAL_W*S,  GH*S); glVertex3f( GW, GH,   gb);
        glTexCoord2f(0.0f,      GH*S); glVertex3f(-GW, GH,   gb);

        // Face lateral esquerda (x = -GW)
        glNormal3f(gz - gb, 0.0f, 0.0f);
        glTexCoord2f(0.0f,  0.0f); glVertex3f(-GW, 0.0f, gz);
        glTexCoord2f(GD*S,  0.0f); glVertex3f(-GW, 0.0f, gb);
        glTexCoord2f(GD*S,  GH*S); glVertex3f(-GW, GH,   gb);
        glTexCoord2f(0.0f,  GH*S); glVertex3f(-GW, GH,   gz);

        // Face lateral direita (x = +GW)
        glNormal3f(gz - gb, 0.0f, 0.0f);
        glTexCoord2f(0.0f,  0.0f); glVertex3f( GW, 0.0f, gz);
        glTexCoord2f(GD*S,  0.0f); glVertex3f( GW, 0.0f, gb);
        glTexCoord2f(GD*S,  GH*S); glVertex3f( GW, GH,   gb);
        glTexCoord2f(0.0f,  GH*S); glVertex3f( GW, GH,   gz);

        // Face superior (y = GH)
        glNormal3f(0.0f, gz - gb, 0.0f);
        glTexCoord2f(0.0f,      0.0f); glVertex3f(-GW, GH, gz);
        glTexCoord2f(GOAL_W*S,  0.0f); glVertex3f( GW, GH, gz);
        glTexCoord2f(GOAL_W*S,  GD*S); glVertex3f( GW, GH, gb);
        glTexCoord2f(0.0f,      GD*S); glVertex3f(-GW, GH, gb);
        glEnd();

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
    };

    drawGoal(-hl, -1.0f);
    drawGoal( hl,  1.0f);

    gluDeleteQuadric(q);
    glLineWidth(1.0f);
}

// Sombra das traves: só a estrutura dos dois gols, para o passe de sombra
// (a rede é fina/transparente e não projeta sombra).
void desenharSombraGols() {
    const float hl = FL / 2.0f;
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_NONE);
    framaGol(q, -hl, -1.0f);
    framaGol(q,  hl,  1.0f);
    gluDeleteQuadric(q);
}
