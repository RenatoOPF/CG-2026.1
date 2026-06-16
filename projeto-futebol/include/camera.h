#pragma once
void setupCamera();

// Move a câmera livre (drone). Cada parâmetro é um eixo de entrada em [-1,1]:
// frente/trás, esquerda/direita, baixo/cima, girar (yaw) e inclinar (pitch).
void moverCameraLivre(float dt, float frente, float lado, float cima,
                      float dyaw, float dpitch);

// Gira a câmera livre por um deslocamento direto do mouse (em pixels).
void girarCameraLivre(float dxPixels, float dyPixels);
