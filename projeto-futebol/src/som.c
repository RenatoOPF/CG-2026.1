#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "som.h"
#include <stdio.h>

ma_engine engine;
int som_iniciado = 0;

void inicializar_som() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("Falha ao inicializar miniaudio.\n");
        som_iniciado = 0;
        return;
    }
    som_iniciado = 1;
    printf("Áudio inicializado com sucesso.\n");
}

void tocar_som_gol() {
    if (!som_iniciado) return;
    ma_engine_play_sound(&engine, "assets/audio/gol.wav", NULL);
}

void tocar_som_chute() {
    if (!som_iniciado) return;
    ma_engine_play_sound(&engine, "assets/audio/chute.wav", NULL);
}

void finalizar_som() {
    if (som_iniciado) {
        ma_engine_uninit(&engine);
    }
}