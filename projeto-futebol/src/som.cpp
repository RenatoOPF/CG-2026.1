#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "som.h"
#include <cstdio>

static ma_engine engine;
static bool      somOk = false;

void initSom() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        fprintf(stderr, "[som] Falha ao inicializar miniaudio.\n");
        return;
    }
    somOk = true;
}

void playApito() {
    if (!somOk) return;
    ma_engine_play_sound(&engine, "assets/audio/apito.wav", NULL);
}

void playGol() {
    if (!somOk) return;
    ma_engine_play_sound(&engine, "assets/audio/gol.wav", NULL);
}

void playChute() {
    if (!somOk) return;
    ma_engine_play_sound(&engine, "assets/audio/chute.wav", NULL);
}
