#ifndef JOGO_H
#define JOGO_H

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
void jogo_atualizar(Jogo *jogo);
void jogo_finalizar(Jogo *jogo);

#endif
