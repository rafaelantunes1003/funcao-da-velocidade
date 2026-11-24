#ifndef DESENHO_H
#define DESENHO_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "jogo.h"

enum {
    BOTAO_MENU = 1,
    BOTAO_REINICIAR,
    BOTAO_PROXIMA_FASE
};

/* Retorna a acao do botao clicado ou zero quando nenhum foi clicado. */
int desenho_identificar_botao(int x, int y, const Jogo *jogo);

/* Desenha a tela inteira: pista, gráfico, carro, obstáculos, mensagem e campo de texto */
void desenhar_tela(const Jogo *jogo, const ALLEGRO_USTR *texto,
                   ALLEGRO_BITMAP *carro, ALLEGRO_FONT *fonte);

#endif
