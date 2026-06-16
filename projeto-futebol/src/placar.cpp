#include "placar.h"
#include "globals.h"
#include "som.h"
#include "iluminacao.h"
#include <GL/glut.h>
#include <cstdio>
#include <cstring>
#include <cmath>

static void texto(const char* s, float x, float y, void* fonte) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; c++)
        glutBitmapCharacter(fonte, *c);
}

void desenharPlacar() {
    // Salva projeção 3D e muda para 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_ENABLE_BIT);   // HUD 2D: sem iluminação, textura ou profundidade
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // --- Fundo do placar (barra superior) ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    glBegin(GL_QUADS);
        glVertex2f(WIN_W / 2.0f - 130, WIN_H - 5);
        glVertex2f(WIN_W / 2.0f + 130, WIN_H - 5);
        glVertex2f(WIN_W / 2.0f + 130, WIN_H - 45);
        glVertex2f(WIN_W / 2.0f - 130, WIN_H - 45);
    glEnd();
    glDisable(GL_BLEND);

    // Placar: "Amarelo 0 x 0 Azul"
    char buf[64];
    snprintf(buf, sizeof(buf), "     Brasil  %d  x  %d  Argentina", gGame.scoreA, gGame.scoreB);
    glColor3f(1.0f, 1.0f, 1.0f);
    texto(buf, WIN_W / 2.0f - 120, WIN_H - 30, GLUT_BITMAP_HELVETICA_18);

    // --- Cronômetro regressivo da partida (abaixo do placar) ---
    int rem = (int)ceilf(MATCH_DURATION - gGame.time);
    if (rem < 0) rem = 0;
    if (rem > (int)MATCH_DURATION) rem = (int)MATCH_DURATION;
    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "%d:%02d", rem / 60, rem % 60);
    glColor3f(1.0f, 1.0f, 0.6f);
    texto(tbuf, WIN_W / 2.0f - 16, WIN_H - 62, GLUT_BITMAP_HELVETICA_18);

    // --- Mensagem de GOL ---
    if (gGame.goalActive && !gGame.matchOver) {
        // Fundo semitransparente
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        glBegin(GL_QUADS);
            glVertex2f(WIN_W / 2.0f - 180, WIN_H / 2.0f - 50);
            glVertex2f(WIN_W / 2.0f + 180, WIN_H / 2.0f - 50);
            glVertex2f(WIN_W / 2.0f + 180, WIN_H / 2.0f + 50);
            glVertex2f(WIN_W / 2.0f - 180, WIN_H / 2.0f + 50);
        glEnd();
        glDisable(GL_BLEND);

        // Pisca em amarelo/branco
        float flash = fabsf(sinf(gGame.time * 6.0f));
        if (gGame.goalTeam == TEAM_A) glColor3f(1.0f, 0.85f * flash, 0.0f);
        else                          glColor3f(0.53f * flash, 0.81f, 0.98f);

        const char* gol = "G O L !";
        int gw = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)gol);
        texto(gol, WIN_W / 2.0f - gw / 2.0f, WIN_H / 2.0f + 10, GLUT_BITMAP_TIMES_ROMAN_24);

        const char* quem = (gGame.goalTeam == TEAM_A) ? "Gol do Brasil!" : "Gol da Argentina!";
        int qw = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)quem);
        glColor3f(1.0f, 1.0f, 1.0f);
        texto(quem, WIN_W / 2.0f - qw / 2.0f, WIN_H / 2.0f - 25, GLUT_BITMAP_HELVETICA_18);
    }

    // --- Resultado final (quando a partida termina) ---
    if (gGame.matchOver) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.72f);
        glBegin(GL_QUADS);
            glVertex2f(WIN_W / 2.0f - 230, WIN_H / 2.0f - 80);
            glVertex2f(WIN_W / 2.0f + 230, WIN_H / 2.0f - 80);
            glVertex2f(WIN_W / 2.0f + 230, WIN_H / 2.0f + 80);
            glVertex2f(WIN_W / 2.0f - 230, WIN_H / 2.0f + 80);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.9f, 0.3f);
        texto("FIM DE JOGO", WIN_W / 2.0f - 78, WIN_H / 2.0f + 42, GLUT_BITMAP_TIMES_ROMAN_24);

        const char* res = (gGame.scoreA > gGame.scoreB) ? "Brasil venceu!"
                        : (gGame.scoreB > gGame.scoreA) ? "Argentina venceu!"
                        :                                 "Empate!";
        // Centraliza pela largura real do texto (funciona para qualquer resultado)
        int rw = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)res);
        glColor3f(1.0f, 1.0f, 1.0f);
        texto(res, WIN_W / 2.0f - rw / 2.0f, WIN_H / 2.0f + 6, GLUT_BITMAP_HELVETICA_18);

        char fbuf[48];
        snprintf(fbuf, sizeof(fbuf), "Brasil  %d  x  %d  Argentina", gGame.scoreA, gGame.scoreB);
        texto(fbuf, WIN_W / 2.0f - 95, WIN_H / 2.0f - 26, GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.8f, 0.8f, 0.8f);
        texto("R: jogar novamente", WIN_W / 2.0f - 62, WIN_H / 2.0f - 56, GLUT_BITMAP_HELVETICA_12);
    }

    // --- Horário atual (canto superior direito) ---
    const char* hora = (gHorario == HORA_TARDE) ? "Horario: Entardecer"
                     : (gHorario == HORA_NOITE) ? "Horario: Noite"
                     :                            "Horario: Manha";
    glColor3f(1.0f, 0.95f, 0.80f);
    texto(hora, WIN_W - 200, WIN_H - 30, GLUT_BITMAP_HELVETICA_18);

    // --- FPS (canto superior esquerdo) ---
    static int   fpsFrames = 0, fpsT0 = 0, fps = 0;
    fpsFrames++;
    int fpsNow = glutGet(GLUT_ELAPSED_TIME);
    if (fpsT0 == 0) fpsT0 = fpsNow;
    if (fpsNow - fpsT0 >= 500) {   // atualiza 2x por segundo
        fps   = (int)(fpsFrames * 1000.0f / (fpsNow - fpsT0) + 0.5f);
        fpsFrames = 0; fpsT0 = fpsNow;
    }
    char fpsbuf[16];
    snprintf(fpsbuf, sizeof(fpsbuf), "FPS: %d", fps);
    glColor3f(0.7f, 1.0f, 0.7f);
    texto(fpsbuf, 10, WIN_H - 30, GLUT_BITMAP_HELVETICA_12);

    // --- HUD inferior: controles e câmera ---
    const char* cams[] = {"[Cam: Bola]", "[Cam: TV]", "[Cam: Aerea]", "[Cam: Livre]"};
    glColor3f(0.9f, 0.9f, 0.9f);
    texto(cams[gCamera], 10, WIN_H - 48, GLUT_BITMAP_HELVETICA_12);
    texto("WASD/Setas: mover bola   C: camera   L: luz   H: horario   R: reiniciar   ESC: sair",
          10, 12, GLUT_BITMAP_HELVETICA_12);

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void verificarGol() {
    if (gGame.goalActive) return;
    const float hl = FL / 2.0f;
    const float gw = GOAL_W / 2.0f;

    // Bola cruza linha sul (z < -hl) dentro do gol → time B marca
    if (gBall.z < -hl && fabsf(gBall.x) < gw) {
        gGame.scoreB++;
        gGame.goalActive = true;
        gGame.goalTimer  = 2.5f;
        gGame.goalTeam   = TEAM_B;
        playGol();
    }
    // Bola cruza linha norte (z > +hl) dentro do gol → time A marca
    else if (gBall.z > hl && fabsf(gBall.x) < gw) {
        gGame.scoreA++;
        gGame.goalActive = true;
        gGame.goalTimer  = 2.5f;
        gGame.goalTeam   = TEAM_A;
        playGol();
    }
}
