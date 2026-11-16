#ifndef CONFIG_H
#define CONFIG_H

#include <allegro5/allegro.h>

/* Janela */
#define LARGURA 800
#define ALTURA  600
#define FPS     60.0

/* Pista: retângulo da tela onde o gráfico é desenhado */
#define PISTA_X1 50
#define PISTA_Y1 30
#define PISTA_X2 770
#define PISTA_Y2 450

/* Parte do gráfico que aparece na pista */
#define X_MIN 0.0
#define X_MAX 10.0
#define Y_MIN -5.0
#define Y_MAX 5.0

/* Campo onde o jogador digita a função */
#define CAMPO_X1 50
#define CAMPO_Y1 520
#define CAMPO_X2 770
#define CAMPO_Y2 565
#define CAMPO_PADDING 10
#define CAMPO_PREFIXO "f(x) = "

/* Carro */
#define CARRO_IMAGEM   "images/carro.png"
#define CARRO_CENTRO_X 334
#define CARRO_CENTRO_Y 191
#define CARRO_ESCALA   0.22f
#define CARRO_DIREITA  ((float)(ALLEGRO_PI / 2))

#define VELOCIDADE 1.5

#endif
