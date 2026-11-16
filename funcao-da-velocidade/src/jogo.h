#ifndef JOGO_H
#define JOGO_H

#include "expressao.h"

typedef enum {
    PARADO,
    ANDANDO,
    CHEGOU,
    SAIU
} Estado;

typedef struct {
    Expressao *funcao;
    char erro[128];
    Estado estado;
    int passos;
    double x;
    double y;
    double x_saida;
    double y_saida;
    float angulo;
} Jogo;

void jogo_iniciar(Jogo *jogo);

/* Compila a função digitada e coloca o carro na largada (tecla ENTER) */
void jogo_largar(Jogo *jogo, const char *texto);

/* Anda o carro um passo (chamada a cada frame) */
void jogo_atualizar(Jogo *jogo);

void jogo_finalizar(Jogo *jogo);

#endif
