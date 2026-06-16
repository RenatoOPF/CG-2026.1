#pragma once

extern bool gLuzOn;   // L: liga/desliga iluminação

// Horário do dia (tecla H alterna em ciclo). Define luz, céu, sol/lua e sombras.
enum Horario { HORA_DIA = 0, HORA_TARDE = 1, HORA_NOITE = 2 };
extern int gHorario;

void initIluminacao();
void aplicarLuzes();   // chamada por frame, após setupCamera()
void atualizarCeu();   // ajusta glClearColor conforme hora do dia
void desenharCeu();    // esfera de céu texturizada (se houver imagem)
void desenharSol();    // disco solar visível no céu

// Sombras planares projetadas no gramado (y=0) na direção da luz principal.
// Uso: if (iniciarSombra()) { ...geometria em modo sombra...; finalizarSombra(); }
bool iniciarSombra();
void finalizarSombra();
