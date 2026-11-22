#ifndef OBSTACULOS_H
#define OBSTACULOS_H

#include <stdbool.h>

typedef enum {
    OBSTACULO_PEDRA,
    OBSTACULO_CONE,
    OBSTACULO_BURACO
} TipoObstaculo;

typedef struct {
    TipoObstaculo tipo;
    double x;
    double y;
    float meia_largura_px;
    float meia_altura_px;
} Obstaculo;

/* Obstáculos são posicionados em coordenadas do gráfico.
 * As dimensões de colisão são definidas em pixels.
 * A lista de obstáculos de cada fase fica em fases.c. */

/* Carrega e libera as imagens dos obstáculos. */
bool obstaculos_inicializar(void);
void obstaculos_finalizar(void);

/* Verifica a colisão do carro ao percorrer o trecho entre dois pontos. */
const Obstaculo *obstaculos_verificar_colisao(const Obstaculo *obstaculos,
                                               int quantidade,
                                               double x_anterior,
                                               double y_anterior,
                                               double x_atual,
                                               double y_atual);

/* Desenha as imagens dos obstáculos. */
void obstaculos_desenhar(const Obstaculo *obstaculos, int quantidade);

#endif
