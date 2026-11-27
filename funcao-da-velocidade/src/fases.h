#ifndef FASES_H
#define FASES_H

#include <stdbool.h>
#include <allegro5/allegro.h>
#include "obstaculos.h"

typedef struct {
    const char *imagem;              /* fundo da pista */
    double (*centro)(double x);      /* caminho certo (onde o carro larga) */
    double (*asfalto)(double x);     /* meio da estrada desenhada na imagem; o carro
                                        sai da pista se ficar a mais de
                                        ASFALTO_MEIA_ALTURA dele */
    const Obstaculo *obstaculos;
    int num_obstaculos;
    const char *instrucao;           /* mensagem antes de largar */
    const char *dica;                /* aparece ao clicar no botao DICA */
} Fase;

/* Carrega e libera as imagens das pistas. */
bool fases_inicializar(void);
void fases_finalizar(void);

int fases_quantidade(void);
const Fase *fases_obter(int indice);
ALLEGRO_BITMAP *fases_imagem(int indice);

/* O ponto (x, y) está em cima do asfalto da fase? */
bool fase_no_asfalto(const Fase *fase, double x, double y);

#endif
