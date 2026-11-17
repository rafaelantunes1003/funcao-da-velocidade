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
 * As dimensões de colisão são definidas em pixels. */
const Obstaculo *obstaculos_obter(int *quantidade);

/* Verifica a colisão do carro ao percorrer o trecho entre dois pontos. */
const Obstaculo *obstaculos_verificar_colisao(double x_anterior,
                                               double y_anterior,
                                               double x_atual,
                                               double y_atual);

/* Desenha as formas provisórias; podem ser substituídas por imagens depois. */
void obstaculos_desenhar(void);

#endif
