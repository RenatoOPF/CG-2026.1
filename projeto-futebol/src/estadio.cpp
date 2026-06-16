#include "estadio.h"
#include "globals.h"
#include "texturas.h"
#include "iluminacao.h"
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

// Um degrau de canto (quarto de anel): piso horizontal + espelho frontal,
// texturizado em concreto. (sx,sz) escolhe o quadrante; o centro do arco é o
// canto interno (sx*SX0, sz*SZ0). ri/ro = raios interno/externo do degrau.
static void arcoDegrau(float sx, float sz, float SX0, float SZ0,
                       float ri, float ro, float y, float stepH) {
    const int SEG = 8;
    const float Cx = sx * SX0, Cz = sz * SZ0;
    for (int s = 0; s < SEG; s++) {
        float a0 = (s     / (float)SEG) * (float)M_PI * 0.5f;
        float a1 = ((s+1) / (float)SEG) * (float)M_PI * 0.5f;
        float c0 = cosf(a0), d0 = sinf(a0), c1 = cosf(a1), d1 = sinf(a1);
        float v0 = a0 * ro / 3.0f, v1 = a1 * ro / 3.0f;   // textura ao longo do arco
        float du = (ro - ri) / 3.0f;
        // Piso (quarto de anel)
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0,  v0); glVertex3f(Cx+sx*ri*c0, y, Cz+sz*ri*d0);
        glTexCoord2f(du, v0); glVertex3f(Cx+sx*ro*c0, y, Cz+sz*ro*d0);
        glTexCoord2f(du, v1); glVertex3f(Cx+sx*ro*c1, y, Cz+sz*ro*d1);
        glTexCoord2f(0,  v1); glVertex3f(Cx+sx*ri*c1, y, Cz+sz*ri*d1);
        glEnd();
        // Espelho (no raio interno, voltado para o centro)
        glBegin(GL_QUADS);
        glNormal3f(-sx*c0, 0, -sz*d0);
        glTexCoord2f(0,          v0); glVertex3f(Cx+sx*ri*c0, y-stepH, Cz+sz*ri*d0);
        glTexCoord2f(0,          v1); glVertex3f(Cx+sx*ri*c1, y-stepH, Cz+sz*ri*d1);
        glTexCoord2f(stepH/3.0f, v1); glVertex3f(Cx+sx*ri*c1, y,       Cz+sz*ri*d1);
        glTexCoord2f(stepH/3.0f, v0); glVertex3f(Cx+sx*ri*c0, y,       Cz+sz*ri*d0);
        glEnd();
    }
}

// Parede externa em arco (quarto de cilindro) no canto, raio Roff, altura H.
static void arcoParede(float sx, float sz, float SX0, float SZ0, float Roff, float H) {
    const int SEG = 8;
    const float Cx = sx * SX0, Cz = sz * SZ0;
    for (int s = 0; s < SEG; s++) {
        float a0 = (s     / (float)SEG) * (float)M_PI * 0.5f;
        float a1 = ((s+1) / (float)SEG) * (float)M_PI * 0.5f;
        float c0 = cosf(a0), d0 = sinf(a0), c1 = cosf(a1), d1 = sinf(a1);
        float v0 = a0 * Roff / 3.0f, v1 = a1 * Roff / 3.0f;
        glBegin(GL_QUADS);
        glNormal3f(sx*c0, 0, sz*d0);   // aponta para fora
        glTexCoord2f(v0, 0);     glVertex3f(Cx+sx*Roff*c0, 0, Cz+sz*Roff*d0);
        glTexCoord2f(v1, 0);     glVertex3f(Cx+sx*Roff*c1, 0, Cz+sz*Roff*d1);
        glTexCoord2f(v1, H/3.0f);glVertex3f(Cx+sx*Roff*c1, H, Cz+sz*Roff*d1);
        glTexCoord2f(v0, H/3.0f);glVertex3f(Cx+sx*Roff*c0, H, Cz+sz*Roff*d0);
        glEnd();
    }
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

    // ============ Arquibancada em tigela (anéis concêntricos) ============
    // Cada degrau é um anel em formato de retângulo arredondado: trechos retos
    // nas laterais (±X) e fundos (±Z) + arcos nos cantos, todos no mesmo passo.
    const int   NB  = 10;             // nº de degraus (fileiras)
    const float STD = 1.3f;           // profundidade de cada degrau
    const float STH = 1.2f;           // altura de cada degrau
    const float SX0 = hw + PISTA_L;   // borda interna em X (≈23.8)
    const float SZ0 = hl + PISTA_E;   // borda interna em Z (35)
    const float SXout = SX0 + NB * STD;   // borda externa em X (36.8)
    const float SZout = SZ0 + NB * STD;   // borda externa em Z (48)
    const float SHout = NB * STH;         // altura total da bancada (12)

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexConcreto);
    glColor3f(0.80f, 0.80f, 0.83f);

    for (int k = 0; k < NB; k++) {
        float di = k * STD, dout = (k + 1) * STD;   // offsets interno/externo
        float xi = SX0 + di, xo = SX0 + dout;
        float zi = SZ0 + di, zo = SZ0 + dout;
        float y  = (k + 1) * STH;
        float vZ = (2.0f * SZ0) / 3.0f, vX = (2.0f * SX0) / 3.0f;

        // Lateral +X (piso + espelho voltado para -X)
        quad4t( xi, y, -SZ0,  xo, y, -SZ0,  xo, y, SZ0,  xi, y, SZ0, STD/3.0f, vZ);
        quad4t( xi, y-STH, -SZ0,  xi, y-STH, SZ0,  xi, y, SZ0,  xi, y, -SZ0, vZ, STH/3.0f);
        // Lateral -X
        quad4t(-xo, y, -SZ0, -xi, y, -SZ0, -xi, y, SZ0, -xo, y, SZ0, STD/3.0f, vZ);
        quad4t(-xi, y-STH, SZ0, -xi, y-STH, -SZ0, -xi, y, -SZ0, -xi, y, SZ0, vZ, STH/3.0f);
        // Fundo +Z
        quad4t(-SX0, y, zi,  SX0, y, zi,  SX0, y, zo, -SX0, y, zo, vX, STD/3.0f);
        quad4t(-SX0, y-STH, zi,  SX0, y-STH, zi,  SX0, y, zi, -SX0, y, zi, vX, STH/3.0f);
        // Fundo -Z
        quad4t(-SX0, y, -zo,  SX0, y, -zo,  SX0, y, -zi, -SX0, y, -zi, vX, STD/3.0f);
        quad4t(-SX0, y, -zi,  SX0, y, -zi,  SX0, y-STH, -zi, -SX0, y-STH, -zi, vX, STH/3.0f);

        // Cantos em arco (mesmo passo dos trechos retos)
        arcoDegrau( 1.0f,  1.0f, SX0, SZ0, di, dout, y, STH);
        arcoDegrau( 1.0f, -1.0f, SX0, SZ0, di, dout, y, STH);
        arcoDegrau(-1.0f,  1.0f, SX0, SZ0, di, dout, y, STH);
        arcoDegrau(-1.0f, -1.0f, SX0, SZ0, di, dout, y, STH);
    }

    // Parede externa da tigela: trechos retos + cantos em arco
    const float ROFF = NB * STD;
    glColor3f(0.72f, 0.72f, 0.75f);
    quad4t( SXout,0,-SZ0,  SXout,0,SZ0,  SXout,SHout,SZ0,  SXout,SHout,-SZ0, (2.0f*SZ0)/3.0f, SHout/3.0f);
    quad4t(-SXout,0,SZ0,  -SXout,0,-SZ0, -SXout,SHout,-SZ0, -SXout,SHout,SZ0, (2.0f*SZ0)/3.0f, SHout/3.0f);
    quad4t(-SX0,0,SZout,   SX0,0,SZout,   SX0,SHout,SZout,  -SX0,SHout,SZout, (2.0f*SX0)/3.0f, SHout/3.0f);
    quad4t( SX0,0,-SZout, -SX0,0,-SZout, -SX0,SHout,-SZout,  SX0,SHout,-SZout, (2.0f*SX0)/3.0f, SHout/3.0f);
    arcoParede( 1.0f,  1.0f, SX0, SZ0, ROFF, SHout);
    arcoParede( 1.0f, -1.0f, SX0, SZ0, ROFF, SHout);
    arcoParede(-1.0f,  1.0f, SX0, SZ0, ROFF, SHout);
    arcoParede(-1.0f, -1.0f, SX0, SZ0, ROFF, SHout);
    glDisable(GL_TEXTURE_2D);

    // Escadas (vomitórios): tiras claras de degraus cortando as bancadas
    const int NAL = 6;   // nas laterais (ao longo de Z)
    const int NAB = 5;   // nos fundos (ao longo de X)
    glColor3f(0.56f, 0.56f, 0.59f);
    for (int i = 0; i < NAL; i++) {
        float za = -SZ0 + (i + 0.5f) * (2.0f * SZ0) / NAL;
        for (int k = 0; k < NB; k++) {
            float xi = SX0 + k * STD, xo = xi + STD, y = (k + 1) * STH + 0.03f;
            quad4( xi, y, za-0.6f,  xo, y, za-0.6f,  xo, y, za+0.6f,  xi, y, za+0.6f);
            quad4(-xo, y, za-0.6f, -xi, y, za-0.6f, -xi, y, za+0.6f, -xo, y, za+0.6f);
        }
    }
    for (int i = 0; i < NAB; i++) {
        float xa = -SX0 + (i + 0.5f) * (2.0f * SX0) / NAB;
        for (int k = 0; k < NB; k++) {
            float zi = SZ0 + k * STD, zo = zi + STD, y = (k + 1) * STH + 0.03f;
            quad4(xa-0.7f, y,  zi, xa+0.7f, y,  zi, xa+0.7f, y,  zo, xa-0.7f, y,  zo);
            quad4(xa-0.7f, y, -zi, xa+0.7f, y, -zi, xa+0.7f, y, -zo, xa-0.7f, y, -zo);
        }
    }
    glColor3f(1.0f, 1.0f, 1.0f);

    // --- Cobertura (telhado) sobre bancadas laterais ---
    // Característica marcante do Rei Pelé: cobertura escura com borda curva interna
    const float RY     = SHout + 5.0f;    // altura do telhado
    const float REXT   = SXout + 2.0f;    // borda externa do telhado
    const float RINT   = SX0 - 2.0f;      // borda interna (sobre o campo)
    const float RZ0    = -SZ0 + 3.0f;
    const float RZ1    =  SZ0 - 3.0f;

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

    // Faixa interna do telhado (acabamento claro pintado — sem luz artificial)
    glColor3f(0.80f, 0.80f, 0.76f);
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

    // Sustentação do telhado: colunas na borda externa indo do CHÃO até o teto,
    // mais uma viga longitudinal e mãos-francesas diagonais até o topo da bancada.
    // (antes os pilares paravam na altura da bancada e o telhado ficava flutuando)
    glColor3f(0.58f, 0.58f, 0.62f);
    for (int s = -1; s <= 1; s += 2) {
        float px = s * REXT;
        for (float pz = RZ0; pz <= RZ1 + 0.01f; pz += 8.0f) {
            // Coluna vertical do chão ao telhado
            glPushMatrix();
            glTranslatef(px, RY / 2.0f, pz);
            glScalef(1.3f, RY, 1.3f);
            glutSolidCube(1.0f);
            glPopMatrix();
            // Mão-francesa: viga diagonal ligando a coluna ao topo da bancada
            float ix = s * SXout;             // topo externo da arquibancada
            float mx = (px + ix) / 2.0f, my = (RY + SHout) / 2.0f;
            float dx = px - ix, dy = RY - SHout;
            float len = sqrtf(dx*dx + dy*dy);
            float ang = atan2f(dy, dx) * 180.0f / (float)M_PI;
            glPushMatrix();
            glTranslatef(mx, my, pz);
            glRotatef(ang, 0.0f, 0.0f, 1.0f);
            glScalef(len, 0.5f, 0.5f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
        // Viga longitudinal no topo das colunas, sob a borda do telhado
        glPushMatrix();
        glTranslatef(px, RY - 0.4f, (RZ0 + RZ1) / 2.0f);
        glScalef(1.5f, 0.8f, RZ1 - RZ0 + 1.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // --- Torres dos cantos (estrutura; sem refletores acesos) ---
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
        // Plataforma de refletores: acesa (emissiva) à noite, apagada de dia
        bool noite = (gHorario == HORA_NOITE);
        if (noite) {
            const GLfloat em[] = {0.95f, 0.90f, 0.70f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
            glColor3f(1.0f, 0.96f, 0.80f);
        } else {
            glColor3f(0.45f, 0.45f, 0.48f);
        }
        glPushMatrix();
        glTranslatef(c[0], TH+0.5f, c[1]);
        glScalef(4.0f, 0.7f, 4.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        if (noite) {
            const GLfloat emZero[] = {0.0f, 0.0f, 0.0f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emZero);
        }
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

// Geometria simplificada (telhado, colunas e torres) para o passe de sombra.
// Recalcula as mesmas dimensões usadas em desenharEstadio.
void desenharSombraEstrutura() {
    const float hw = FW / 2.0f, hl = FL / 2.0f;
    const float PISTA_L = 3.8f, PISTA_E = 5.0f;
    const float SX0 = hw + PISTA_L, SZ0 = hl + PISTA_E;
    const int   NB = 10; const float STD = 1.3f, STH = 1.2f;
    const float SXout = SX0 + NB * STD, SZout = SZ0 + NB * STD, SHout = NB * STH;
    const float RY = SHout + 5.0f, REXT = SXout + 2.0f, RINT = SX0 - 2.0f;
    const float RZ0 = -SZ0 + 3.0f, RZ1 = SZ0 - 3.0f;
    const float TH = RY + 10.0f;

    // Painel do telhado (esquerdo e direito), projetado como quad plano
    glBegin(GL_QUADS);
    glVertex3f(-RINT, RY, RZ0); glVertex3f(-REXT, RY, RZ0);
    glVertex3f(-REXT, RY, RZ1); glVertex3f(-RINT, RY, RZ1);
    glVertex3f( REXT, RY, RZ0); glVertex3f( RINT, RY, RZ0);
    glVertex3f( RINT, RY, RZ1); glVertex3f( REXT, RY, RZ1);
    glEnd();

    // Colunas de sustentação na borda externa
    for (int s = -1; s <= 1; s += 2) {
        float px = s * REXT;
        for (float pz = RZ0; pz <= RZ1 + 0.01f; pz += 8.0f) {
            glPushMatrix();
            glTranslatef(px, RY / 2.0f, pz);
            glScalef(1.3f, RY, 1.3f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    // Torres dos cantos
    float corners[4][2] = { {-SXout-2.0f,-SZout-2.0f},{SXout+2.0f,-SZout-2.0f},
                            {-SXout-2.0f, SZout+2.0f},{SXout+2.0f, SZout+2.0f} };
    for (auto& c : corners) {
        glPushMatrix();
        glTranslatef(c[0], TH / 2.0f, c[1]);
        glScalef(1.2f, TH, 1.2f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}
