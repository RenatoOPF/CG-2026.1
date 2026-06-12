#include "estadio.h"
#include "globals.h"
#include "texturas.h"
#include <GL/glut.h>
#include <cmath>

// Quad com normal calculada automaticamente a partir do winding
// (TWO_SIDE corrige a face vista pelo outro lado)
static void quad4(float x0,float y0,float z0,
                  float x1,float y1,float z1,
                  float x2,float y2,float z2,
                  float x3,float y3,float z3) {
    float ax = x1-x0, ay = y1-y0, az = z1-z0;
    float bx = x3-x0, by = y3-y0, bz = z3-z0;
    float nx = ay*bz - az*by;
    float ny = az*bx - ax*bz;
    float nz = ax*by - ay*bx;
    glBegin(GL_QUADS);
    glNormal3f(nx, ny, nz);   // GL_NORMALIZE renormaliza
    glVertex3f(x0,y0,z0); glVertex3f(x1,y1,z1);
    glVertex3f(x2,y2,z2); glVertex3f(x3,y3,z3);
    glEnd();
}

// Quad texturizado: texcoords (0,0)..(u,v) nos cantos
static void quad4t(float x0,float y0,float z0,
                   float x1,float y1,float z1,
                   float x2,float y2,float z2,
                   float x3,float y3,float z3,
                   float u, float v) {
    float ax = x1-x0, ay = y1-y0, az = z1-z0;
    float bx = x3-x0, by = y3-y0, bz = z3-z0;
    float nx = ay*bz - az*by;
    float ny = az*bx - ax*bz;
    float nz = ax*by - ay*bx;
    glBegin(GL_QUADS);
    glNormal3f(nx, ny, nz);
    glTexCoord2f(0,0); glVertex3f(x0,y0,z0);
    glTexCoord2f(u,0); glVertex3f(x1,y1,z1);
    glTexCoord2f(u,v); glVertex3f(x2,y2,z2);
    glTexCoord2f(0,v); glVertex3f(x3,y3,z3);
    glEnd();
}

// Degrau de arquibancada: face horizontal + face vertical (texturizado)
static void degrau(float x0, float x1, float y, float z0, float z1, float dy, float dz) {
    float uw = (x1 - x0) / 3.0f, vl = (z1 - z0) / 3.0f;
    // Assento (horizontal)
    quad4t(x0,y,z0, x1,y,z0, x1,y,z1, x0,y,z1, uw, vl);
    // Espelho (vertical, frente do degrau)
    quad4t(x0,y,z0, x1,y,z0, x1,y-dy,z0-dz, x0,y-dy,z0-dz, uw, dy/3.0f);
}

void desenharEstadio() {
    const float hw = FW / 2.0f;    // 20
    const float hl = FL / 2.0f;    // 30

    // Dimensões da pista oval (flat, terracota)
    const float PISTA_L = 3.8f;    // largura lateral
    const float PISTA_E = 5.0f;    // profundidade end

    // Pista de atletismo — terracota texturizada
    const float PTS = 3.0f;        // 1 tile a cada 3u
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexPista);
    glColor3f(1.0f, 1.0f, 1.0f);
    // Lados
    quad4t(-hw-PISTA_L,0.01f,-hl,   -hw,0.01f,-hl,   -hw,0.01f, hl,   -hw-PISTA_L,0.01f, hl,
           PISTA_L/PTS, FL/PTS);
    quad4t( hw,0.01f,-hl,    hw+PISTA_L,0.01f,-hl,    hw+PISTA_L,0.01f, hl,  hw,0.01f, hl,
           PISTA_L/PTS, FL/PTS);
    // Extremidades (fundo)
    quad4t(-hw-PISTA_L,0.01f,-hl-PISTA_E, hw+PISTA_L,0.01f,-hl-PISTA_E,
            hw+PISTA_L,0.01f,-hl,         -hw-PISTA_L,0.01f,-hl,
           (FW+2*PISTA_L)/PTS, PISTA_E/PTS);
    quad4t(-hw-PISTA_L,0.01f, hl,          hw+PISTA_L,0.01f, hl,
            hw+PISTA_L,0.01f, hl+PISTA_E, -hw-PISTA_L,0.01f, hl+PISTA_E,
           (FW+2*PISTA_L)/PTS, PISTA_E/PTS);
    glDisable(GL_TEXTURE_2D);

    // Borda interna da pista (linha branca fina, sem iluminação)
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-hw-PISTA_L,0.05f,-hl-PISTA_E);
    glVertex3f( hw+PISTA_L,0.05f,-hl-PISTA_E);
    glVertex3f( hw+PISTA_L,0.05f, hl+PISTA_E);
    glVertex3f(-hw-PISTA_L,0.05f, hl+PISTA_E);
    glEnd();
    glLineWidth(1.0f);
    glPopAttrib();

    // --- Arquibancadas laterais com degraus (concreto texturizado) ---
    const int NDEG = 10;
    const float DEG_W = 1.3f;   // largura do degrau (profundidade da bancada)
    const float DEG_H = 1.2f;   // altura do degrau
    const float SX0  = hw + PISTA_L;  // borda interna da bancada lateral (≈23.8)
    const float z0 = -hl - PISTA_E;
    const float z1 =  hl + PISTA_E;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexConcreto);
    glColor3f(0.80f, 0.80f, 0.83f);

    for (int k = 0; k < NDEG; k++) {
        float xInner = SX0 + k * DEG_W;
        float xOuter = xInner + DEG_W;
        float yTop   = (k + 1) * DEG_H;

        // Arquibancada esquerda (x negativo): cresce para -x
        degrau(-xOuter, -xInner, yTop, z0, z1, DEG_H, DEG_W);
        // Arquibancada direita (x positivo): cresce para +x
        degrau( xInner,  xOuter, yTop, z0, z1, DEG_H, DEG_W);
    }

    // Parede externa vertical das bancadas laterais
    float SXout = SX0 + NDEG * DEG_W;
    float SHout = NDEG * DEG_H;
    glColor3f(0.72f, 0.72f, 0.75f);
    quad4t(-SXout,0,z0, -SXout,0,z1, -SXout,SHout,z1, -SXout,SHout,z0, (z1-z0)/3.0f, SHout/3.0f);
    quad4t( SXout,0,z1,  SXout,0,z0,  SXout,SHout,z0,  SXout,SHout,z1, (z1-z0)/3.0f, SHout/3.0f);

    // --- Arquibancadas de fundo (atrás dos gols) com degraus ---
    const int NDEG_E = 7;
    const float DEG_WE = 1.4f;
    const float DEG_HE = 1.1f;
    const float SZ0 = hl + PISTA_E;    // borda interna fundo
    const float xE0 = -SXout, xE1 = SXout;

    glColor3f(0.80f, 0.80f, 0.83f);
    for (int k = 0; k < NDEG_E; k++) {
        float zInner = SZ0 + k * DEG_WE;
        float zOuter = zInner + DEG_WE;
        float yTop   = (k + 1) * DEG_HE;
        float uw = (xE1 - xE0) / 3.0f;
        // Sul (z negativo)
        quad4t(xE0, yTop, -(zInner), xE1, yTop, -(zInner),
               xE1, yTop, -(zOuter), xE0, yTop, -(zOuter), uw, DEG_WE/3.0f);  // assento
        quad4t(xE0, yTop, -(zInner), xE1, yTop, -(zInner),
               xE1, yTop-DEG_HE, -(zInner)+DEG_WE, xE0, yTop-DEG_HE, -(zInner)+DEG_WE,
               uw, DEG_HE/3.0f); // espelho
        // Norte (z positivo)
        quad4t(xE0, yTop, zInner, xE1, yTop, zInner,
               xE1, yTop, zOuter, xE0, yTop, zOuter, uw, DEG_WE/3.0f);
        quad4t(xE0, yTop, zInner, xE1, yTop, zInner,
               xE1, yTop-DEG_HE, zInner-DEG_WE, xE0, yTop-DEG_HE, zInner-DEG_WE,
               uw, DEG_HE/3.0f);
    }
    // Parede fundo
    float SZout = SZ0 + NDEG_E * DEG_WE;
    float SEHout = NDEG_E * DEG_HE;
    glColor3f(0.72f, 0.72f, 0.75f);
    quad4t(xE0, 0, -SZout, xE1, 0, -SZout, xE1, SEHout, -SZout, xE0, SEHout, -SZout,
           (xE1-xE0)/3.0f, SEHout/3.0f);
    quad4t(xE0, 0,  SZout, xE1, 0,  SZout, xE1, SEHout,  SZout, xE0, SEHout,  SZout,
           (xE1-xE0)/3.0f, SEHout/3.0f);
    // Laterais do bloco fundo
    quad4t(-SXout,0,-SZ0, -SXout,0,-SZout, -SXout,SEHout,-SZout, -SXout,SEHout,-SZ0,
           (SZout-SZ0)/3.0f, SEHout/3.0f);
    quad4t( SXout,0,-SZout,  SXout,0,-SZ0,  SXout,SEHout,-SZ0,  SXout,SEHout,-SZout,
           (SZout-SZ0)/3.0f, SEHout/3.0f);
    quad4t(-SXout,0, SZ0,  -SXout,0, SZout, -SXout,SEHout, SZout, -SXout,SEHout, SZ0,
           (SZout-SZ0)/3.0f, SEHout/3.0f);
    quad4t( SXout,0, SZout,  SXout,0, SZ0,   SXout,SEHout, SZ0,   SXout,SEHout, SZout,
           (SZout-SZ0)/3.0f, SEHout/3.0f);
    glDisable(GL_TEXTURE_2D);

    // --- Cobertura (telhado) sobre bancadas laterais ---
    // Característica marcante do Rei Pelé: cobertura escura com borda curva interna
    const float RY     = SHout + 5.0f;    // altura do telhado
    const float REXT   = SXout + 2.0f;    // borda externa do telhado
    const float RINT   = SX0 - 2.0f;      // borda interna (sobre o campo)
    const float RZ0    = z0 + 3.0f;
    const float RZ1    = z1 - 3.0f;

    // Painel principal do telhado (escuro, quase preto — cor característica do Rei Pelé)
    glColor3f(0.13f, 0.13f, 0.14f);
    // Telhado esquerdo
    quad4(-RINT, RY, RZ0, -REXT, RY, RZ0, -REXT, RY, RZ1, -RINT, RY, RZ1);
    // Telhado direito
    quad4( REXT, RY, RZ0,  RINT, RY, RZ0,  RINT, RY, RZ1,  REXT, RY, RZ1);

    // Borda frontal do telhado (aba que desce levemente — perfil característico)
    const float ABA = 2.2f;
    glColor3f(0.10f, 0.10f, 0.11f);
    quad4(-RINT, RY, RZ0, -RINT, RY, RZ1, -RINT, RY-ABA, RZ1, -RINT, RY-ABA, RZ0);
    quad4( RINT, RY, RZ1,  RINT, RY, RZ0,  RINT, RY-ABA, RZ0,  RINT, RY-ABA, RZ1);

    // Faixa branca interna do telhado (lâmpadas — material emissivo)
    const GLfloat emLampada[] = {0.85f, 0.83f, 0.72f, 1.0f};
    const GLfloat emZero[]    = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emLampada);
    glColor3f(0.95f, 0.95f, 0.90f);
    const int NL = 12;
    for (int i = 0; i < NL; i++) {
        float t  = (float)i / NL;
        float t2 = (float)(i+1) / NL;
        float za = RZ0 + t  * (RZ1 - RZ0);
        float zb = RZ0 + t2 * (RZ1 - RZ0);
        if (i % 2 == 0) {
            quad4(-RINT-0.3f, RY-0.05f, za, -RINT-1.8f, RY-0.05f, za,
                  -RINT-1.8f, RY-0.05f, zb, -RINT-0.3f, RY-0.05f, zb);
            quad4( RINT+1.8f, RY-0.05f, za,  RINT+0.3f, RY-0.05f, za,
                   RINT+0.3f, RY-0.05f, zb,  RINT+1.8f, RY-0.05f, zb);
        }
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emZero);

    // Pilares do telhado
    glColor3f(0.62f, 0.62f, 0.65f);
    for (float pz = RZ0+3.0f; pz <= RZ1-3.0f; pz += 10.0f) {
        glPushMatrix();
        glTranslatef(-SXout, SHout/2.0f, pz);
        glScalef(1.0f, SHout, 1.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(SXout, SHout/2.0f, pz);
        glScalef(1.0f, SHout, 1.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // --- Torres de iluminação (4 cantos) ---
    const float TH = RY + 10.0f;
    glColor3f(0.72f, 0.72f, 0.72f);
    float corners[4][2] = { {-SXout-2.0f,-SZout-2.0f},{SXout+2.0f,-SZout-2.0f},
                             {-SXout-2.0f, SZout+2.0f},{SXout+2.0f, SZout+2.0f} };
    for (auto& c : corners) {
        // Poste
        glPushMatrix();
        glTranslatef(c[0], TH/2.0f, c[1]);
        glScalef(1.2f, TH, 1.2f);
        glutSolidCube(1.0f);
        glPopMatrix();
        // Plataforma de refletores (emissiva: fonte de luz visível)
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emLampada);
        glColor3f(1.0f, 0.96f, 0.82f);
        glPushMatrix();
        glTranslatef(c[0], TH+0.5f, c[1]);
        glScalef(4.0f, 0.7f, 4.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emZero);
        glColor3f(0.72f, 0.72f, 0.72f);
    }

    // --- Chão externo ---
    glColor3f(0.36f, 0.32f, 0.28f);
    const float GND = SXout + 30.0f;
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex3f(-GND,-0.1f, GND);
        glVertex3f( GND,-0.1f, GND);
        glVertex3f( GND,-0.1f,-GND);
        glVertex3f(-GND,-0.1f,-GND);
    glEnd();
}
