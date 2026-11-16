#ifndef CAMPO_TEXTO_H
#define CAMPO_TEXTO_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

/* Acrescenta uma letra, se ainda couber no campo */
void campo_digitar(ALLEGRO_USTR *texto, int letra, ALLEGRO_FONT *fonte);

void campo_apagar_ultima_letra(ALLEGRO_USTR *texto);

/* Desenha a caixa com "f(x) = ", o texto e o cursor piscando */
void campo_desenhar(const ALLEGRO_USTR *texto, ALLEGRO_FONT *fonte);

#endif
