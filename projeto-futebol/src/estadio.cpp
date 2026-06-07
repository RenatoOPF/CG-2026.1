#include "estadio.h"
#include "globals.h"
#include <GL/glut.h>
#include <cmath>

static void quad4(float x0,float y0,float z0,
                  float x1,float y1,float z1,
                  float x2,float y2,float z2,
                  float x3,float y3,float z3) {
    glBegin(GL_QUADS);
    glVertex3f(x0,y0,z0); glVertex3f(x1,y1,z1);
    glVertex3f(x2,y2,z2); glVertex3f(x3,y3,z3);
    glEnd();
}

// Degrau de arquibancada: face horizontal + face vertical
static void degrau(float x0, float x1, float y, float z0, float z1, float dy, float dz) {
    // Assento (horizontal)
    quad4(x0,y,z0, x1,y,z0, x1,y,z1, x0,y,z1);
    // Espelho (vertical, frente do degrau)
    quad4(x0,y,z0, x1,y,z0, x1,y-dy,z0-dz, x0,y-dy,z0-dz);
}

void desenharEstadio() {
    const float hw = FW / 2.0f;    // 20
    const float hl = FL / 2.0f;    // 30

    // Dimensões da pista oval (flat, terracota)
    const float PISTA_L = 3.8f;    // largura lateral
    const float PISTA_E = 5.0f;    // profundidade end

    // Pista de atletismo — terracota
    glColor3f(0.70f, 0.33f, 0.16f);
    // Lados
    quad4(-hw-PISTA_L,0.01f,-hl,   -hw,0.01f,-hl,   -hw,0.01f, hl,   -hw-PISTA_L,0.01f, hl);
    quad4( hw,0.01f,-hl,    hw+PISTA_L,0.01f,-hl,    hw+PISTA_L,0.01f, hl,  hw,0.01f, hl);
    // Extremidades (fundo)
    quad4(-hw-PISTA_L,0.01f,-hl-PISTA_E, hw+PISTA_L,0.01f,-hl-PISTA_E,
           hw+PISTA_L,0.01f,-hl,         -hw-PISTA_L,0.01f,-hl);
    quad4(-hw-PISTA_L,0.01f, hl,          hw+PISTA_L,0.01f, hl,
           hw+PISTA_L,0.01f, hl+PISTA_E, -hw-PISTA_L,0.01f, hl+PISTA_E);

    // Borda interna da pista (linha branca fina)
    glColor3f(1,1,1);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-hw-PISTA_L,0.05f,-hl-PISTA_E);
    glVertex3f( hw+PISTA_L,0.05f,-hl-PISTA_E);
    glVertex3f( hw+PISTA_L,0.05f, hl+PISTA_E);
    glVertex3f(-hw-PISTA_L,0.05f, hl+PISTA_E);
    glEnd();
    glLineWidth(1.0f);

    // --- Arquibancadas laterais com degraus ---
    // Parâmetros: 10 degraus, cada degrau 1.1u largo e 1.2u alto
    const int NDEG = 10;
    const float DEG_W = 1.3f;   // largura do degrau (profundidade da bancada)
    const float DEG_H = 1.2f;   // altura do degrau
    const float SX0  = hw + PISTA_L;  // borda interna da bancada lateral (≈23.8)
    // cor concreto / cimento
    const float z0 = -hl - PISTA_E;
    const float z1 =  hl + PISTA_E;

    glColor3f(0.55f, 0.55f, 0.58f);

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
    glColor3f(0.50f, 0.50f, 0.52f);
    quad4(-SXout,0,z0, -SXout,0,z1, -SXout,SHout,z1, -SXout,SHout,z0);
    quad4( SXout,0,z1,  SXout,0,z0,  SXout,SHout,z0,  SXout,SHout,z1);

    // --- Arquibancadas de fundo (atrás dos gols) com degraus ---
    const int NDEG_E = 7;
    const float DEG_WE = 1.4f;
    const float DEG_HE = 1.1f;
    const float SZ0 = hl + PISTA_E;    // borda interna fundo
    const float xE0 = -SXout, xE1 = SXout;

    glColor3f(0.55f, 0.55f, 0.58f);
    for (int k = 0; k < NDEG_E; k++) {
        float zInner = SZ0 + k * DEG_WE;
        float zOuter = zInner + DEG_WE;
        float yTop   = (k + 1) * DEG_HE;
        // Sul (z negativo)
        quad4(xE0, yTop, -(zInner), xE1, yTop, -(zInner),
              xE1, yTop, -(zOuter), xE0, yTop, -(zOuter));  // assento
        quad4(xE0, yTop, -(zInner), xE1, yTop, -(zInner),
              xE1, yTop-DEG_HE, -(zInner)+DEG_WE, xE0, yTop-DEG_HE, -(zInner)+DEG_WE); // espelho
        // Norte (z positivo)
        quad4(xE0, yTop, zInner, xE1, yTop, zInner,
              xE1, yTop, zOuter, xE0, yTop, zOuter);
        quad4(xE0, yTop, zInner, xE1, yTop, zInner,
              xE1, yTop-DEG_HE, zInner-DEG_WE, xE0, yTop-DEG_HE, zInner-DEG_WE);
    }
    // Parede fundo
    float SZout = SZ0 + NDEG_E * DEG_WE;
    float SEHout = NDEG_E * DEG_HE;
    glColor3f(0.50f, 0.50f, 0.52f);
    quad4(xE0, 0, -SZout, xE1, 0, -SZout, xE1, SEHout, -SZout, xE0, SEHout, -SZout);
    quad4(xE0, 0,  SZout, xE1, 0,  SZout, xE1, SEHout,  SZout, xE0, SEHout,  SZout);
    // Laterais do bloco fundo
    quad4(-SXout,0,-SZ0, -SXout,0,-SZout, -SXout,SEHout,-SZout, -SXout,SEHout,-SZ0);
    quad4( SXout,0,-SZout,  SXout,0,-SZ0,  SXout,SEHout,-SZ0,  SXout,SEHout,-SZout);
    quad4(-SXout,0, SZ0,  -SXout,0, SZout, -SXout,SEHout, SZout, -SXout,SEHout, SZ0);
    quad4( SXout,0, SZout,  SXout,0, SZ0,   SXout,SEHout, SZ0,   SXout,SEHout, SZout);

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

    // Faixa branca interna do telhado (lâmpadas)
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
        // Plataforma de refletores
        glColor3f(1.0f, 0.96f, 0.82f);
        glPushMatrix();
        glTranslatef(c[0], TH+0.5f, c[1]);
        glScalef(4.0f, 0.7f, 4.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        glColor3f(0.72f, 0.72f, 0.72f);
    }

    // --- Chão externo ---
    glColor3f(0.36f, 0.32f, 0.28f);
    const float GND = SXout + 30.0f;
    quad4(-GND,-0.1f,-GND, GND,-0.1f,-GND, GND,-0.1f,GND, -GND,-0.1f,GND);
}
