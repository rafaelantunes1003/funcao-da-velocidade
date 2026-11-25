#ifndef DESENHO_H
#define DESENHO_H

#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "jogo.h"

enum {
    BOTAO_MENU = 1,
    BOTAO_REINICIAR,
    BOTAO_PROXIMA_FASE,
    BOTAO_SELECIONAR_FASES
};

/* Retorna a acao do botao clicado ou zero quando nenhum foi clicado. */
int desenho_identificar_botao(int x, int y, const Jogo *jogo);

/* Tela de selecao: retorna o indice da fase clicada ou -1. */
int desenho_identificar_fase(int x, int y);
bool desenho_clicou_voltar_fases(int x, int y);
void desenho_tela_fases(int fase_atual, ALLEGRO_FONT *fonte);

/* Desenha a tela inteira: pista, gráfico, carro, obstáculos, mensagem e campo de texto */
void desenhar_tela(const Jogo *jogo, const ALLEGRO_USTR *texto,
                   ALLEGRO_BITMAP *carro, ALLEGRO_FONT *fonte);

#endif
