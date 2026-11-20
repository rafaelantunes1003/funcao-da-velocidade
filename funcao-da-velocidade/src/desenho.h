#ifndef DESENHO_H
#define DESENHO_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "jogo.h"

/* Desenha a tela inteira: pista, gráfico, carro, obstáculos, mensagem e campo de texto */
void desenhar_tela(const Jogo *jogo, const ALLEGRO_USTR *texto,
                   ALLEGRO_BITMAP *carro, ALLEGRO_BITMAP *pista, ALLEGRO_FONT *fonte);

#endif
