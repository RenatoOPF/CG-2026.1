#include "bola.h"
#include "globals.h"
#include "som.h"
#include <cmath>

struct V3 { float x, y, z; };
static V3 v3n(V3 v) {
    float l = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return {v.x/l, v.y/l, v.z/l};
}

// out = A * B, todas as matrizes 4x4 em column-major (formato do OpenGL)
static void mulMat(float* out, const float* A, const float* B) {
    float r[16];
    for (int c = 0; c < 4; c++)
        for (int row = 0; row < 4; row++) {
            float s = 0.0f;
            for (int k = 0; k < 4; k++) s += A[k*4 + row] * B[c*4 + k];
            r[c*4 + row] = s;
        }
    for (int i = 0; i < 16; i++) out[i] = r[i];
}

// Acumula uma rotação (eixo unitário, ângulo) na orientação da bola, aplicada
// no referencial do mundo: rot = R * rot. (matriz de Rodrigues em column-major)
static void rotacionar(float ax, float ay, float az, float ang) {
    float c = cosf(ang), s = sinf(ang), t = 1.0f - c;
    float R[16] = {
        c + ax*ax*t,     ax*ay*t + az*s,  ax*az*t - ay*s,  0.0f,
        ax*ay*t - az*s,  c + ay*ay*t,     ay*az*t + ax*s,  0.0f,
        ax*az*t + ay*s,  ay*az*t - ax*s,  c + az*az*t,     0.0f,
        0.0f,            0.0f,            0.0f,            1.0f
    };
    mulMat(gBall.rot, R, gBall.rot);
}

static void drawFace(const V3* pts, int n, float r, float cr, float cg, float cb) {
    V3 c = {0,0,0};
    for (int i = 0; i < n; i++) { c.x+=pts[i].x; c.y+=pts[i].y; c.z+=pts[i].z; }
    c = v3n({c.x/n, c.y/n, c.z/n});
    glColor3f(cr, cg, cb);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(c.x, c.y, c.z);
    glVertex3f(r*c.x, r*c.y, r*c.z);
    for (int i = 0; i <= n; i++) {
        const V3& p = pts[i % n];
        glNormal3f(p.x, p.y, p.z);
        glVertex3f(r*p.x, r*p.y, r*p.z);
    }
    glEnd();
}

// Truncated icosahedron: 12 black pentagons + 20 white hexagons
void desenharBola() {
    const float PHI = (1.0f + sqrtf(5.0f)) / 2.0f;
    const float r = BALL_R;

    // Icosahedron vertices (normalized)
    V3 IV[12] = {
        { 0, 1, PHI},{ 0,-1, PHI},{ 0, 1,-PHI},{ 0,-1,-PHI},
        { 1, PHI, 0},{-1, PHI, 0},{ 1,-PHI, 0},{-1,-PHI, 0},
        { PHI, 0, 1},{ PHI, 0,-1},{-PHI, 0, 1},{-PHI, 0,-1}
    };
    for (int i = 0; i < 12; i++) IV[i] = v3n(IV[i]);

    // 20 triangular faces (CCW from outside)
    int IF[20][3] = {
        {0,4,8},{0,8,1},{0,1,10},{0,10,5},{0,5,4},
        {4,9,8},{8,9,6},{8,6,1},{1,6,7},{1,7,10},
        {10,7,11},{10,11,5},{5,11,2},{5,2,4},{4,2,9},
        {3,9,2},{3,2,11},{3,11,7},{3,7,6},{3,6,9}
    };

    // Cyclic neighbor ring for each vertex (derived from face traversal)
    int pent[12][5] = {
        {4,5,10,1,8}, {0,10,7,6,8}, {5,4,9,3,11}, {9,6,7,11,2},
        {8,9,2,5,0},  {0,4,2,11,10},{8,1,7,3,9},  {10,11,3,6,1},
        {0,1,6,9,4},  {8,6,3,2,4},  {0,5,11,7,1}, {5,2,3,7,10}
    };

    // T[a][b] = normalized point on edge (a,b) at 1/3 from a
    V3 T[12][12] = {};
    for (int f = 0; f < 20; f++) {
        for (int e = 0; e < 3; e++) {
            int u = IF[f][e], v = IF[f][(e+1)%3];
            T[u][v] = v3n({(2*IV[u].x+IV[v].x)/3,(2*IV[u].y+IV[v].y)/3,(2*IV[u].z+IV[v].z)/3});
            T[v][u] = v3n({(IV[u].x+2*IV[v].x)/3,(IV[u].y+2*IV[v].y)/3,(IV[u].z+2*IV[v].z)/3});
        }
    }

    glPushMatrix();
    glTranslatef(gBall.x, gBall.y, gBall.z);
    glMultMatrixf(gBall.rot);   // orientação acumulada da rolagem

    // 20 white hexagons
    for (int f = 0; f < 20; f++) {
        int a = IF[f][0], b = IF[f][1], c = IF[f][2];
        V3 hex[6] = {T[a][b], T[b][a], T[b][c], T[c][b], T[c][a], T[a][c]};
        drawFace(hex, 6, r, 1.0f, 1.0f, 1.0f);
    }

    // 12 black pentagons
    for (int v = 0; v < 12; v++) {
        V3 pen[5];
        for (int k = 0; k < 5; k++) pen[k] = T[v][pent[v][k]];
        drawFace(pen, 5, r, 0.05f, 0.05f, 0.05f);
    }

    // Draw seam lines between faces
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);
    for (int f = 0; f < 20; f++) {
        int a = IF[f][0], b = IF[f][1], c = IF[f][2];
        glBegin(GL_LINE_LOOP);
        glVertex3f(r*T[a][b].x, r*T[a][b].y, r*T[a][b].z);
        glVertex3f(r*T[b][a].x, r*T[b][a].y, r*T[b][a].z);
        glVertex3f(r*T[b][c].x, r*T[b][c].y, r*T[b][c].z);
        glVertex3f(r*T[c][b].x, r*T[c][b].y, r*T[c][b].z);
        glVertex3f(r*T[c][a].x, r*T[c][a].y, r*T[c][a].z);
        glVertex3f(r*T[a][c].x, r*T[a][c].y, r*T[a][c].z);
        glEnd();
    }

    glPopMatrix();
}

// Geometria simples para a projeção de sombra (a cor vem do passe de sombra)
void desenharSombraBola() {
    glPushMatrix();
    glTranslatef(gBall.x, gBall.y, gBall.z);
    glutSolidSphere(BALL_R, 12, 12);
    glPopMatrix();
}

void atualizarBola(float dt, float dirX, float dirZ) {
    float tvx = dirX * BALL_MAX_SPEED;
    float tvz = dirZ * BALL_MAX_SPEED;
    float lf  = 1.0f - powf(0.08f, dt);
    gBall.vx  = gBall.vx + (tvx - gBall.vx) * lf;
    gBall.vz  = gBall.vz + (tvz - gBall.vz) * lf;

    gBall.x += gBall.vx * dt;
    gBall.z += gBall.vz * dt;

    // Rolagem sem deslizamento: gira em torno do eixo horizontal perpendicular
    // ao movimento, com ângulo = distância percorrida / raio.
    float sp = sqrtf(gBall.vx*gBall.vx + gBall.vz*gBall.vz);
    if (sp > 0.01f) {
        rotacionar(gBall.vz / sp, 0.0f, -gBall.vx / sp, sp * dt / BALL_R);
    }

    const float hw = FW / 2.0f - BALL_R;
    const float hl = FL / 2.0f;

    if (gBall.x >  hw) { gBall.x =  hw; gBall.vx = -fabsf(gBall.vx) * 0.6f; }
    if (gBall.x < -hw) { gBall.x = -hw; gBall.vx =  fabsf(gBall.vx) * 0.6f; }

    if (gBall.z < -hl && fabsf(gBall.x) > GOAL_W / 2.0f) {
        gBall.z = -hl; gBall.vz = fabsf(gBall.vz) * 0.6f;
    }
    if (gBall.z >  hl && fabsf(gBall.x) > GOAL_W / 2.0f) {
        gBall.z = hl;  gBall.vz = -fabsf(gBall.vz) * 0.6f;
    }
    if (gBall.z < -(hl + GOAL_D + BALL_R)) { gBall.z = -(hl + GOAL_D); gBall.vz = 0; }
    if (gBall.z >   hl + GOAL_D + BALL_R)  { gBall.z =   hl + GOAL_D;  gBall.vz = 0; }
}
