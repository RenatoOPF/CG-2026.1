CC = gcc
CFLAGS = -Wall
LDFLAGS = -lGL -lGLU -lglut

# Encontra todos os arquivos .c dentro da pasta src
SRCS = $(wildcard src/*.c)

# Gera o nome dos executáveis baseados nos arquivos .c encontrados (remove o "src/" e o ".c")
TARGETS = $(patsubst src/%.c,%,$(SRCS))

all: $(TARGETS)

# Regra genérica: para gerar um executável, compile o respectivo .c de dentro da pasta src
%: src/%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)