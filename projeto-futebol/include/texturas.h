#pragma once
#include <GL/glut.h>

extern GLuint gTexGrama;
extern GLuint gTexConcreto;
extern GLuint gTexPista;
extern GLuint gTexRede;
extern GLuint gTexCeu;        // céu de meio-dia
extern GLuint gTexCeuTarde;   // céu de entardecer
extern GLuint gTexCeuNoite;   // céu de noite

// Carrega uma textura de arquivo (.jpg/.png). Retorna 0 se o arquivo faltar.
GLuint carregarTexturaArquivo(const char* caminho, bool mipmap);

void initTexturas();
