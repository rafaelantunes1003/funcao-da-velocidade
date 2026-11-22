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
/* Borda final da faixa quadriculada da imagem da pista. */
/* Centro da faixa quadriculada; a vitória também exige estar sobre o asfalto. */
#define X_CHEGADA 8.35
/* Distância máxima (para cima ou para baixo) entre o carro e a linha amarela
 * do meio da pista para o carro continuar no asfalto. */
#define ASFALTO_MEIA_ALTURA 2.5
#define Y_MIN -5.0
#define Y_MAX 5.0
/* Escala vertical ampliada para a fase 2, cuja resposta-alvo é y = x + 5. */
#define FASE2_Y_MIN 4.0
#define FASE2_Y_MAX 17.0
#define FASE3_Y_MIN -3.5
#define FASE3_Y_MAX 4.0
#define FASE3_ASFALTO_MEIA_ALTURA 1.0

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
