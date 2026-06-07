#include "jogadores.h"
#include "globals.h"
#include "som.h"
#include <GL/glut.h>
#include <cmath>
#include <algorithm>

static const float formA[TEAM_SIZE][3] = {
    { 0.0f, -28.0f, 1},
    {-8.0f, -18.0f, 0},
    { 8.0f, -18.0f, 0},
    {-5.0f,  -8.0f, 0},
    { 5.0f,  -8.0f, 0},
};
static const float formB[TEAM_SIZE][3] = {
    { 0.0f,  28.0f, 1},
    {-8.0f,  18.0f, 0},
    { 8.0f,  18.0f, 0},
    {-5.0f,   8.0f, 0},
    { 5.0f,   8.0f, 0},
};

void initJogadores() {
    gPlayers.clear();
    for (int i = 0; i < TEAM_SIZE; i++) {
        Player p;
        p.x = p.homeX = formA[i][0];
        p.z = p.homeZ = formA[i][1];
        p.team = TEAM_A;
        p.isGK = formA[i][2] > 0.5f;
        gPlayers.push_back(p);
    }
    for (int i = 0; i < TEAM_SIZE; i++) {
        Player p;
        p.x = p.homeX = formB[i][0];
        p.z = p.homeZ = formB[i][1];
        p.team = TEAM_B;
        p.isGK = formB[i][2] > 0.5f;
        gPlayers.push_back(p);
    }
}

static void desenharJogador(const Player& p) {
    float r, g, b;
    if (p.isGK) {
        // Goleiro team A = verde; team B = preto
        if (p.team == TEAM_A) { r = 0.10f; g = 0.65f; b = 0.15f; }
        else                  { r = 0.10f; g = 0.10f; b = 0.10f; }
    } else {
        if (p.team == TEAM_A) { r = 1.0f;  g = 0.85f; b = 0.0f;  }
        else                  { r = 0.53f; g = 0.81f; b = 0.98f; }
    }

    const float LH = PLR_HEIGHT * 0.42f;
    const float BH = PLR_HEIGHT * 0.40f;
    const float HR = PLR_BW * 0.55f;
    const float LW = PLR_BW * 0.22f;

    glPushMatrix();
    glTranslatef(p.x, 0.0f, p.z);
    glRotatef(p.facing, 0.0f, 1.0f, 0.0f);

    // Pernas
    glColor3f(0.15f, 0.15f, 0.5f);
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * PLR_BW * 0.25f, LH / 2.0f, 0.0f);
        glRotatef(side * p.legAngle, 1.0f, 0.0f, 0.0f);
        glScalef(LW * 2.0f, LH, LW * 2.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Tronco
    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(0.0f, LH + BH / 2.0f, 0.0f);
    glScalef(PLR_BW, BH, PLR_BW * 0.65f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Braços
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * (PLR_BW * 0.65f), LH + BH * 0.75f, 0.0f);
        glRotatef(-side * p.legAngle * 0.8f, 1.0f, 0.0f, 0.0f);
        glScalef(LW * 1.6f, BH * 0.7f, LW * 1.6f);
        glColor3f(r, g, b);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Cabeça
    glColor3f(0.95f, 0.78f, 0.65f);
    glPushMatrix();
    glTranslatef(0.0f, LH + BH + HR, 0.0f);
    glutSolidSphere(HR, 8, 8);
    glPopMatrix();

    glPopMatrix();
}

void desenharJogadores() {
    for (const auto& p : gPlayers)
        desenharJogador(p);
}

void atualizarIA(float dt) {
    int idxA = -1, idxB = -1;
    float dA = 1e9f, dB = 1e9f;

    for (int i = 0; i < (int)gPlayers.size(); i++) {
        if (gPlayers[i].isGK) continue;
        float dx = gPlayers[i].x - gBall.x;
        float dz = gPlayers[i].z - gBall.z;
        float d  = sqrtf(dx*dx + dz*dz);
        if (gPlayers[i].team == TEAM_A && d < dA) { dA = d; idxA = i; }
        if (gPlayers[i].team == TEAM_B && d < dB) { dB = d; idxB = i; }
    }

    const float GK_TOUCH = BALL_R + PLR_BW * 0.9f;
    const float hl = FL / 2.0f;

    for (int i = 0; i < (int)gPlayers.size(); i++) {
        Player& p = gPlayers[i];
        if (p.kickCooldown > 0.0f) p.kickCooldown -= dt;

        if (p.isGK) {
            // Goleiro move-se apenas na extensão das traves (±GOAL_W/2)
            float gzHome  = (p.team == TEAM_A) ? -(hl - 1.5f) : (hl - 1.5f);
            float targetX = std::max(-GOAL_W / 2.0f, std::min(GOAL_W / 2.0f, gBall.x));
            float lf = 1.0f - powf(0.01f, dt);
            p.x += (targetX - p.x) * lf;
            p.z += (gzHome  - p.z) * lf;
            p.legAngle = 0.0f;
            p.facing   = (p.team == TEAM_A) ? 0.0f : 180.0f;

            // Colisão com a bola
            float gdx = gBall.x - p.x;
            float gdz = gBall.z - p.z;
            float gd  = sqrtf(gdx*gdx + gdz*gdz);
            if (gd < GK_TOUCH && gd > 0.001f) {
                // Empurra bola para o meio do campo
                float mz = (p.team == TEAM_A) ? 1.0f : -1.0f;
                float pushX = (gdx / gd) * 0.5f;
                float pushZ = mz * 0.866f;
                float pm = sqrtf(pushX*pushX + pushZ*pushZ);
                gBall.vx = (pushX / pm) * BALL_MAX_SPEED * 0.75f;
                gBall.vz = (pushZ / pm) * BALL_MAX_SPEED * 0.75f;
                // Posiciona bola fora do raio de colisão
                gBall.x = p.x + (gdx / gd) * GK_TOUCH * 1.1f;
                gBall.z = p.z + (gdz / gd) * GK_TOUCH * 1.1f;
                playChute();
            }
            continue;
        }

        bool isChaser = (i == idxA || i == idxB);
        p.isChasing   = isChaser;

        float bdx   = gBall.x - p.x;
        float bdz   = gBall.z - p.z;
        float bdist = sqrtf(bdx*bdx + bdz*bdz);
        const float TOUCH = BALL_R + PLR_BW * 0.7f;

        if (isChaser && p.kickCooldown <= 0.0f) {
            if (bdist > TOUCH) {
                p.x += (bdx / bdist) * PLR_SPEED * dt;
                p.z += (bdz / bdist) * PLR_SPEED * dt;
                p.facing   = atan2f(bdx, bdz) * 180.0f / (float)M_PI;
                p.legAngle = 20.0f * sinf(gGame.time * 9.0f + i);
            } else {
                float goalZ = (p.team == TEAM_A) ? FL / 2.0f : -FL / 2.0f;
                float kx = gBall.x - p.x;
                float kz = goalZ - gBall.z;
                float km = sqrtf(kx*kx + kz*kz);
                if (km > 0.01f) {
                    gBall.vx = (kx / km) * BALL_MAX_SPEED * 0.85f;
                    gBall.vz = (kz / km) * BALL_MAX_SPEED * 0.85f;
                    playChute();
                }
                p.kickCooldown = 0.75f;
                if (bdist > 0.01f) {
                    p.x -= (bdx / bdist) * PLR_SPEED * 0.3f * dt;
                    p.z -= (bdz / bdist) * PLR_SPEED * 0.3f * dt;
                }
                p.legAngle = 30.0f;
            }
        } else {
            float hx = p.homeX - p.x;
            float hz = p.homeZ - p.z;
            float hd = sqrtf(hx*hx + hz*hz);
            if (hd > 0.5f) {
                float spd = (p.kickCooldown > 0.0f) ? PLR_SPEED * 0.6f : PLR_SPEED * 0.35f;
                p.x += (hx / hd) * spd * dt;
                p.z += (hz / hd) * spd * dt;
                p.facing   = atan2f(hx, hz) * 180.0f / (float)M_PI;
                p.legAngle = 10.0f * sinf(gGame.time * 5.0f + i);
            } else {
                p.legAngle = 3.0f * sinf(gGame.time * 2.0f + i);
            }
        }
    }

    // Separação entre jogadores
    const float MIN_SEP = PLR_BW * 3.0f;
    for (int i = 0; i < (int)gPlayers.size(); i++) {
        for (int j = i + 1; j < (int)gPlayers.size(); j++) {
            float sx = gPlayers[j].x - gPlayers[i].x;
            float sz = gPlayers[j].z - gPlayers[i].z;
            float sd = sqrtf(sx*sx + sz*sz);
            if (sd < MIN_SEP && sd > 0.001f) {
                float push = (MIN_SEP - sd) / 2.0f;
                gPlayers[i].x -= (sx / sd) * push;
                gPlayers[i].z -= (sz / sd) * push;
                gPlayers[j].x += (sx / sd) * push;
                gPlayers[j].z += (sz / sd) * push;
            }
        }
    }
}
