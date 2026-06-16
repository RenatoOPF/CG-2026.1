# Futebol 3D — Estádio Rei Pelé

Simulação 3D de uma partida de futebol num estádio, feita em **C++ com OpenGL
de pipeline fixo** (GLUT/GLU, modo imediato). Projeto da disciplina de
Computação Gráfica da UFAL (2026.1) — AB1 (modelagem, jogo) + AB2 (iluminação,
texturas e refinamento de modelagem, com foco em realismo).

## Como compilar e rodar

```bash
make          # compila para obj/ e gera o executável futebol_cg
./futebol_cg  # ou: make run
```

Dependências: `g++` (C++17), OpenGL, GLU e **freeglut** (`-lGL -lGLU -lglut`).
As bibliotecas `miniaudio.h` (áudio) e `stb_image.h` (imagens) já vêm como
headers em `include/`.

O `Makefile` usa rastreamento automático de dependências de headers
(`-MMD -MP`), então alterar qualquer `.h` recompila o necessário com um `make`
normal — sem precisar de `make clean`.

## Controles

| Tecla | Ação |
|---|---|
| `W A S D` / setas | Mover a bola |
| `C` | Alternar câmera (Bola / TV / Aérea) |
| `F` | Câmera livre (drone): `WASD` voa, `Espaço`/`Z` sobe-desce, **mouse** olha |
| `L` | Liga/desliga a iluminação |
| `H` | Horário: meio-dia → entardecer → noite |
| `R` | Reiniciar a partida |
| `ESC` | Sair |

> Há um atalho não documentado (`T`) que adianta o relógio para faltarem 5s —
> útil para testar a tela de fim de jogo.

## Como funciona

Dois callbacks do GLUT dirigem o programa:

- **`timer()`** (~60 Hz): calcula o `dt`, processa o relógio da partida, lê o
  teclado, move a câmera livre, atualiza a física da bola e a IA dos jogadores,
  e verifica gol. Congela o jogo quando a partida termina.
- **`display()`**: desenha a cena na ordem **céu → sol → estádio → torcida →
  campo → passe de sombras → jogadores → bola → placar/HUD**.

## Estrutura dos arquivos

| Arquivo | Responsabilidade |
|---|---|
| `main.cpp` | Loop, entrada, ordem de desenho, relógio da partida |
| `globals.h` | Estado e constantes compartilhados (`Ball`, `Player`, `GameState`, câmera) |
| `campo.cpp` | Gramado texturizado, linhas/círculos, traves (cilindros GLU) + rede, sombra das traves |
| `estadio.cpp` | Arquibancada em tigela, pista, telhado, torres, chão externo, sombra da estrutura |
| `torcida.cpp` | Torcedores nas bancadas e cantos, com "ola" senoidal |
| `jogadores.cpp` | Jogadores (corpo de primitivas) e IA de perseguir/chutar |
| `bola.cpp` | Bola (icosaedro truncado), física e rolagem |
| `camera.cpp` | 4 câmeras: Bola, TV, Aérea, Livre (drone) |
| `iluminacao.cpp` | Sol/luar único, céu, disco solar, sombras planares |
| `texturas.cpp` | Carregador de imagem + texturas procedurais (fallback) |
| `placar.cpp` | HUD 2D: placar, cronômetro, FPS, mensagens, resultado; detecção de gol |
| `som.cpp` | Apito, gol e chute (miniaudio) |

## Técnicas implementadas

**Texturas** — `texturas.cpp` tenta carregar imagens de `assets/textures/`
(grama, concreto, pista, rede e os céus). Se o arquivo não existir, usa uma
textura **procedural** gerada por código (ruído tileável). Mapeamento em
`GL_MODULATE` para a luz multiplicar a textura.

**Iluminação (fonte única: o sol)** — uma luz direcional cuja direção alimenta
ao mesmo tempo a luz, a projeção de sombra e o disco solar visível, mantendo
tudo coerente. Três horários (`H`): **meio-dia / entardecer / noite**. À noite o
luar é fraco e azulado e entram os **4 refletores** das torres (única luz
artificial, só nesse horário). A intensidade é calibrada para não saturar o
gramado.

**Céu** — esfera grande texturizada com imagem panorâmica equiretangular,
trocada conforme o horário, desenhada sem luz e sem escrever profundidade. O
disco do **sol/lua** é desenhado como esferas aditivas (núcleo + halo).

**Sombras** — projeção planar no gramado (`y=0`) na direção da luz, com
**stencil** para não escurecer em dobro nas sobreposições. Projetam sombra:
jogadores, bola, traves e a estrutura (telhado, colunas e torres).

**Modelagem da arquibancada** — uma **tigela contínua**: anéis concêntricos em
formato de retângulo arredondado (lados retos + cantos em arco), com escadas
(vomitórios), telhado apoiado em colunas do chão ao teto, e torres nos cantos.

**Câmeras** — Bola (atrás da bola), TV (lateral, seguindo a jogada), Aérea (de
cima) e Livre/drone (voo livre com mouse-look).

**Bola** — geometria de bola de futebol (icosaedro truncado: pentágonos pretos
e hexágonos brancos) com **rolagem sem deslizamento** acumulada por matriz de
rotação.

**Jogadores e torcida** — jogadores montados de primitivas com animação de
pernas e IA simples (o mais próximo de cada time persegue a bola e chuta ao
gol). A torcida faz uma "ola" senoidal.

**Partida e HUD** — placar que detecta gol pela linha, **cronômetro
regressivo** (`MATCH_DURATION`, padrão 60s), **FPS** no canto, mensagens de gol
e **tela de resultado** ao fim (vitória ou empate).

## Assets

Em `assets/`:

- `textures/` — `grama.jpg`, `concreto.jpg`, `pista.jpg`, e os céus
  `ceu.jpg` (meio-dia), `ceu_entardecer.jpg`, `ceu_noite.jpg`. São JPG
  panorâmicos 2:1 (equiretangular) para os céus; opcionais — sem eles, o
  projeto roda com texturas procedurais.
- `audio/` — `apito.wav`, `gol.wav`, `chute.wav`.
