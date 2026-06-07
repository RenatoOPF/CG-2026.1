#include "camera.h"
#include "globals.h"
#include <cmath>

static float tvX    =  0.0f;
static float tvZ    = -32.0f;
static float ballCamDir = -1.0f;  // lerpa entre -1 e +1
float        gCamSign   = -1.0f;

void setupCamera() {
    const float dt = 1.0f / 60.0f;
    const float lf = 1.0f - powf(0.001f, dt);   // ~0.5s suave
    const float lf_fast = 1.0f - powf(0.01f, dt); // ~0.1s rápido (para câmera TV seguir bola)

    switch (gCamera) {

        case CAM_BALL: {
            // Câmera atrás da bola, mais próxima
            float alvo = (gBall.z >= 0.0f) ? -1.0f : 1.0f;
            ballCamDir += (alvo - ballCamDir) * lf;
            gCamSign = (ballCamDir > 0.0f) ? 1.0f : -1.0f;

            float offZ = ballCamDir * 7.0f;   // ~7u atrás (antes era 16)
            float offY = 3.5f;
            float offX = -0.8f;
            gluLookAt(
                gBall.x + offX, gBall.y + offY, gBall.z + offZ,
                gBall.x,        gBall.y,         gBall.z,
                0.0f, 1.0f, 0.0f
            );
            break;
        }

        case CAM_TV: {
            // Câmera TV: segue o X da bola lateralmente e o Z de forma contínua
            float alvoX = gBall.x * 0.25f;          // acompanha levemente a posição X
            float alvoZ = (gBall.z >= 0.0f) ? -34.0f : 34.0f;
            // Desloca câmera na direção Z conforme bola se afasta do centro
            float fracZ = gBall.z / (FL / 2.0f);    // -1..1
            alvoZ -= fracZ * 5.0f;                   // movimento contínuo além da troca de lado

            tvX += (alvoX - tvX) * lf_fast;
            tvZ += (alvoZ - tvZ) * lf;

            gCamSign = (tvZ > 0.0f) ? 1.0f : -1.0f;

            // Olha levemente para a bola no eixo Z também
            float lookZ = gBall.z * 0.15f;
            gluLookAt(
                tvX - 18.0f, 14.0f, tvZ,
                tvX * 0.3f,   0.0f, lookZ,
                0.0f, 1.0f, 0.0f
            );
            break;
        }

        case CAM_TOP:
            gluLookAt(0.0f, 110.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
            break;
    }
}
