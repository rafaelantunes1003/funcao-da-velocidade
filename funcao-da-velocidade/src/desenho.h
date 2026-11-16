#ifndef DESENHO_H
#define DESENHO_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "jogo.h"

/* Desenha a tela inteira: grade, gráfico, carro, mensagem e campo de texto */
void desenhar_tela(const Jogo *jogo, const ALLEGRO_USTR *texto,
                   ALLEGRO_BITMAP *carro, ALLEGRO_FONT *fonte);

#endif
