#include "texturas.h"
#include <vector>

GLuint gTexGrama    = 0;
GLuint gTexConcreto = 0;
GLuint gTexPista    = 0;
GLuint gTexRede     = 0;

// Hash determinístico por texel: gera ruído estável sem depender de rand()
static unsigned hash2(unsigned x, unsigned y) {
    unsigned h = x * 374761393u + y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

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

static GLuint texGrama() {
    const int N = 128;
    std::vector<unsigned char> px(N * N * 3);
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            unsigned h = hash2(x, y);
            int v = (int)(h % 25) - 12;                 // ruído -12..12
            int r = 42 + v, g = 134 + v, b = 42 + v;
            if (h % 89 == 0) { g += 30; r += 12; }      // "folhas" mais claras
            unsigned char* p = &px[(y * N + x) * 3];
            p[0] = (unsigned char)r; p[1] = (unsigned char)g; p[2] = (unsigned char)b;
        }
    return criarTextura(N, N, px.data(), GL_RGB, true);
}

static GLuint texConcreto() {
    const int N = 128;
    std::vector<unsigned char> px(N * N * 3);
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            unsigned h = hash2(x + 777, y);
            int v = (int)(h % 25) - 12;
            int c = 150 + v;
            if (h % 113 == 0) c -= 35;                  // pontos escuros
            unsigned char* p = &px[(y * N + x) * 3];
            p[0] = (unsigned char)c; p[1] = (unsigned char)c; p[2] = (unsigned char)(c + 4);
        }
    return criarTextura(N, N, px.data(), GL_RGB, true);
}

static GLuint texPista() {
    const int N = 128;
    std::vector<unsigned char> px(N * N * 3);
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            unsigned h = hash2(x, y + 555);
            int v = (int)(h % 19) - 9;
            unsigned char* p = &px[(y * N + x) * 3];
            p[0] = (unsigned char)(182 + v);
            p[1] = (unsigned char)(86 + v / 2);
            p[2] = (unsigned char)(46 + v / 2);
        }
    return criarTextura(N, N, px.data(), GL_RGB, true);
}

// Rede do gol: grade branca com furos transparentes (RGBA + alpha test).
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
    gTexGrama    = texGrama();
    gTexConcreto = texConcreto();
    gTexPista    = texPista();
    gTexRede     = texRede();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
