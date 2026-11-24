#ifndef JOGO_H
#define JOGO_H

#include <stdbool.h>
#include "expressao.h"
#include "obstaculos.h"

typedef enum {
    PARADO,
    ANDANDO,
    CHEGOU,
    SAIU,
    COLIDIU
} Estado;

typedef struct {
    int fase;                 /* 0 = fase 1, 1 = fase 2... */
    Expressao *funcao;
    char erro[128];
    Estado estado;
    int passos;
    double x, y;
    double x_saida, y_saida;
    float angulo;
    const Obstaculo *obstaculo_colisao;
} Jogo;

void jogo_iniciar(Jogo *jogo);
void jogo_largar(Jogo *jogo, const char *texto);
void jogo_reiniciar_fase(Jogo *jogo);
void jogo_atualizar(Jogo *jogo);

/* Depois de vencer: existe uma próxima fase? */
bool jogo_tem_proxima_fase(const Jogo *jogo);

/* Passa para a próxima fase, com o carro parado na largada */
void jogo_proxima_fase(Jogo *jogo);

void jogo_finalizar(Jogo *jogo);

#endif
