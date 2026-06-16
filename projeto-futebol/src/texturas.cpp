#include "texturas.h"
#include <vector>
#include <cmath>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint gTexGrama    = 0;
GLuint gTexConcreto = 0;
GLuint gTexPista    = 0;
GLuint gTexRede     = 0;
GLuint gTexCeu      = 0;
GLuint gTexCeuTarde = 0;

// Hash determinístico por texel: gera ruído estável sem depender de rand()
static unsigned hash2(unsigned x, unsigned y) {
    unsigned h = x * 374761393u + y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static unsigned char clampb(int v) {
    return (unsigned char)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

// Ruído de valor suave e PERIÓDICO (tileável): interpola bilinearmente uma
// grade de valores aleatórios cujo índice é tomado em módulo `period`. Como
// `period` divide a largura da textura, as bordas casam sem emenda no GL_REPEAT.
static float vnoise(float x, float y, int period) {
    int xi = (int)floorf(x), yi = (int)floorf(y);
    float fx = x - xi, fy = y - yi;
    auto g = [&](int X, int Y) {
        unsigned h = hash2((unsigned)((X % period + period) % period),
                           (unsigned)((Y % period + period) % period));
        return (h & 0xffff) / 65535.0f;
    };
    float a = g(xi, yi),     b = g(xi + 1, yi);
    float c = g(xi, yi + 1), d = g(xi + 1, yi + 1);
    fx = fx * fx * (3.0f - 2.0f * fx);   // smoothstep p/ transições macias
    fy = fy * fy * (3.0f - 2.0f * fy);
    float ab = a + (b - a) * fx;
    float cd = c + (d - c) * fx;
    return ab + (cd - ab) * fy;
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

// Carrega uma textura de arquivo (.jpg/.png/...) via stb_image.
// Retorna 0 se o arquivo não existir — quem chama pode cair no procedural.
GLuint carregarTexturaArquivo(const char* caminho, bool mipmap) {
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);   // origem do GL é canto inferior esq.
    unsigned char* data = stbi_load(caminho, &w, &h, &n, 0);
    if (!data) {
        fprintf(stderr, "[textura] nao encontrada, usando procedural: %s\n", caminho);
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

static GLuint texGrama() {
    const int N = 256;
    std::vector<unsigned char> px(N * N * 3);
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            float u = x / (float)N, v = y / (float)N;

            // Tufos: ruído multi-frequência (8/16/32 dividem N → tileável) cria
            // manchas largas mais claras e mais escuras, como grama real.
            float clump = 0.50f * vnoise(u * 8,  v * 8,  8)
                        + 0.30f * vnoise(u * 16, v * 16, 16)
                        + 0.20f * vnoise(u * 32, v * 32, 32);
            int var = (int)((clump - 0.5f) * 60.0f);

            int r = 38 + var / 2;
            int g = 116 + var;
            int b = 40 + var / 3;

            // Lâminas individuais: estrias verticais finas (x mais denso que y)
            float blade = (hash2((unsigned)(x * 3), (unsigned)y) & 0xff) / 255.0f;
            if (blade > 0.88f)      { r += 16; g += 26; b += 8;  }  // ponta iluminada
            else if (blade < 0.07f) { r -= 14; g -= 22; b -= 10; }  // sombra entre folhas

            // Raras manchas de terra/grama gasta
            if (vnoise(u * 6 + 10, v * 6 + 3, 6) > 0.83f) {
                r = 96 + var / 2; g = 74 + var / 2; b = 50 + var / 3;
            }

            unsigned char* p = &px[(y * N + x) * 3];
            p[0] = clampb(r); p[1] = clampb(g); p[2] = clampb(b);
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

// Tenta carregar a textura do arquivo; se não existir, usa a procedural.
static GLuint texArquivoOuProcedural(const char* caminho, bool mipmap,
                                     GLuint (*procedural)()) {
    GLuint id = carregarTexturaArquivo(caminho, mipmap);
    return id ? id : procedural();
}

void initTexturas() {
    // Basta colocar o arquivo em assets/textures/ que ele substitui a procedural.
    // Use imagens seamless/tileáveis para grama, concreto e pista.
    gTexGrama    = texArquivoOuProcedural("assets/textures/grama.jpg",    true,  texGrama);
    gTexConcreto = texArquivoOuProcedural("assets/textures/concreto.jpg", true,  texConcreto);
    gTexPista    = texArquivoOuProcedural("assets/textures/pista.jpg",    true,  texPista);
    // Rede precisa de alpha (PNG) e sem mipmap, p/ não sumir de longe
    gTexRede     = texArquivoOuProcedural("assets/textures/rede.png",     false, texRede);
    // Céu: imagem panorâmica (JPG equiretangular 2:1). Sem fallback procedural —
    // se faltar, gTexCeu fica 0 e o céu usa só a cor de fundo (glClearColor).
    gTexCeu      = carregarTexturaArquivo("assets/textures/ceu.jpg", true);
    gTexCeuTarde = carregarTexturaArquivo("assets/textures/ceu_entardecer.jpg", true);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
