#include "camera.h"
#include "globals.h"
#include <cmath>

static float tvX    =  0.0f;
static float tvZ    = -32.0f;
static float ballCamDir = -1.0f;  // lerpa entre -1 e +1
float        gCamSign   = -1.0f;

// --- Câmera livre (drone): posição + orientação em ângulos de Euler ---
static float fcX = 0.0f, fcY = 22.0f, fcZ = 55.0f;  // começa alto, atrás do gol sul
static float fcYaw   = 0.0f;     // 0° = olhando para -Z (centro do campo)
static float fcPitch = -18.0f;   // levemente inclinada para baixo

void moverCameraLivre(float dt, float frente, float lado, float cima,
                      float dyaw, float dpitch) {
    fcYaw   += dyaw   * 90.0f * dt;   // 90°/s de rotação
    fcPitch += dpitch * 70.0f * dt;
    if (fcPitch >  89.0f) fcPitch =  89.0f;   // trava p/ não "capotar"
    if (fcPitch < -89.0f) fcPitch = -89.0f;

    float yawR = fcYaw * (float)M_PI / 180.0f;
    // Vetor frente no plano horizontal e vetor direita (perpendicular)
    float fx = sinf(yawR), fz = -cosf(yawR);
    float rx = -fz,        rz = fx;
    const float SPD = 28.0f;          // velocidade de voo (u/s)
    fcX += (fx * frente + rx * lado) * SPD * dt;
    fcZ += (fz * frente + rz * lado) * SPD * dt;
    fcY += cima * SPD * dt;
    if (fcY < 0.5f) fcY = 0.5f;       // não atravessa o chão
}

void girarCameraLivre(float dxPixels, float dyPixels) {
    const float SENS = 0.12f;         // graus por pixel
    fcYaw   += dxPixels * SENS;
    fcPitch -= dyPixels * SENS;       // mouse p/ baixo = olhar p/ baixo
    if (fcPitch >  89.0f) fcPitch =  89.0f;
    if (fcPitch < -89.0f) fcPitch = -89.0f;
}

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

        case CAM_FREE: {
            // Direção do olhar a partir de yaw/pitch (mesma convenção do drone)
            float yawR = fcYaw * (float)M_PI / 180.0f;
            float pitchR = fcPitch * (float)M_PI / 180.0f;
            float dx =  sinf(yawR) * cosf(pitchR);
            float dy =  sinf(pitchR);
            float dz = -cosf(yawR) * cosf(pitchR);
            gluLookAt(fcX, fcY, fcZ, fcX + dx, fcY + dy, fcZ + dz, 0.0f, 1.0f, 0.0f);
            break;
        }
    }
}
