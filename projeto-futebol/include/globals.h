#pragma once
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdio>
#include <cstring>

// Janela
const int WIN_W = 1280;
const int WIN_H  = 720;

// Campo (1 unidade ≈ 1.5m real)
const float FW = 40.0f;   // largura (X: -20..20) ≈ 65m
const float FL = 60.0f;   // comprimento (Z: -30..30) ≈ 100m

// Gol
const float GOAL_W = 4.8f;   // largura total
const float GOAL_H = 1.6f;   // altura
const float GOAL_D = 1.5f;   // profundidade

// Bola
const float BALL_R         = 0.4f;
const float BALL_MAX_SPEED = 20.0f;

// Jogadores
const int   TEAM_SIZE  = 5;
const float PLR_SPEED  = 12.0f;
const float PLR_HEIGHT = 1.8f;
const float PLR_BW     = 0.4f;

const int TEAM_A = 0;  // Amarelo
const int TEAM_B = 1;  // Azul claro

enum CameraMode { CAM_BALL = 0, CAM_TV = 1, CAM_TOP = 2 };

struct Ball {
    float x = 0.0f, y = BALL_R, z = 0.0f;
    float vx = 0.0f, vz = 0.0f;
};

struct Player {
    float x, y = 0.0f, z;
    float homeX, homeZ;
    int   team;
    bool  isChasing    = false;
    float facing       = 0.0f;
    float legAngle     = 0.0f;
    bool  isGK         = false;
    float kickCooldown = 0.0f;  // segundos até poder chutar de novo
};

struct GameState {
    int   scoreA     = 0;
    int   scoreB     = 0;
    bool  goalActive = false;
    float goalTimer  = 0.0f;
    int   goalTeam   = -1;
    float time       = 0.0f;
};

extern Ball                gBall;
extern std::vector<Player> gPlayers;
extern GameState           gGame;
extern CameraMode          gCamera;
extern float               gCamSign;  // +1 ou -1: sinal dos controles conforme lado da câmera TV
