#include "texturas.h"
#include <vector>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint gTexGrama    = 0;
GLuint gTexConcreto = 0;
GLuint gTexPista    = 0;
GLuint gTexRede     = 0;
GLuint gTexCeu      = 0;
GLuint gTexCeuTarde = 0;
GLuint gTexCeuNoite = 0;

static GLuint criarTextura(int w, int h, const unsigned char* px,
                           GLenum fmt, bool mipmap) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int comp = (fmt == GL_RGBA) ? 4 : 3;
    if (mipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, comp, w, h, fmt, GL_UNSIGNED_BYTE, px);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, fmt, GL_UNSIGNED_BYTE, px);
    }
    return id;
}

// Carrega uma textura de arquivo (.jpg/.png/...) via stb_image.
GLuint carregarTexturaArquivo(const char* caminho, bool mipmap) {
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);   // origem do GL é canto inferior esq.
    unsigned char* data = stbi_load(caminho, &w, &h, &n, 0);
    if (!data) {
        fprintf(stderr, "[textura] FALHA ao carregar: %s\n", caminho);
        return 0;
    }
    GLenum fmt;
    if (n == 4)      fmt = GL_RGBA;
    else if (n == 3) fmt = GL_RGB;
    else {                                   // 1 ou 2 canais: recarrega como RGB
        stbi_image_free(data);
        data = stbi_load(caminho, &w, &h, &n, 3);
        fmt  = GL_RGB;
    }
    GLuint id = criarTextura(w, h, data, fmt, mipmap);
    stbi_image_free(data);
    fprintf(stderr, "[textura] carregada %s (%dx%d, %d canais)\n", caminho, w, h, n);
    return id;
}

// Rede do gol: textura procedural RGBA — grade branca com furos transparentes
// (usada com alpha test). É gerada em código, pois não há imagem para ela.
// Sem mipmap: a média dos mipmaps reduziria o alpha e a rede sumiria de longe.
static GLuint texRede() {
    const int N = 64, L = 6;   // L = espessura da corda em texels
    std::vector<unsigned char> px(N * N * 4);
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            bool corda = (x < L) || (y < L);
            unsigned char* p = &px[(y * N + x) * 4];
            p[0] = p[1] = p[2] = 240;
            p[3] = corda ? 255 : 0;
        }
    return criarTextura(N, N, px.data(), GL_RGBA, false);
}

void initTexturas() {
    gTexGrama    = carregarTexturaArquivo("assets/textures/grama.jpg",    true);
    gTexConcreto = carregarTexturaArquivo("assets/textures/concreto.jpg", true);
    gTexPista    = carregarTexturaArquivo("assets/textures/pista.jpg",    true);
    gTexRede     = texRede();   // procedural (sem imagem correspondente)
    // Céu: imagens panorâmicas (JPG equiretangular 2:1) por horário
    gTexCeu      = carregarTexturaArquivo("assets/textures/ceu.jpg", true);
    gTexCeuTarde = carregarTexturaArquivo("assets/textures/ceu_entardecer.jpg", true);
    gTexCeuNoite = carregarTexturaArquivo("assets/textures/ceu_noite.jpg", true);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
