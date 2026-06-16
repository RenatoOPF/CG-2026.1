#pragma once

extern bool gLuzOn;   // L: liga/desliga iluminação
extern bool gNoite;   // N: alterna dia (sol) / noite (refletores)

void initIluminacao();
void aplicarLuzes();   // chamada por frame, após setupCamera()
void atualizarCeu();   // ajusta glClearColor conforme hora do dia
void desenharCeu();    // esfera de céu texturizada (se houver imagem)
void desenharSol();    // disco solar visível no céu

// Sombras planares projetadas no gramado (y=0) na direção da luz principal.
// Uso: if (iniciarSombra()) { ...geometria em modo sombra...; finalizarSombra(); }
bool iniciarSombra();
void finalizarSombra();
