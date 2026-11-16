#include <allegro5/allegro_primitives.h>
#include "campo_texto.h"
#include "config.h"

void campo_apagar_ultima_letra(ALLEGRO_USTR *texto)
{
    int pos = (int)al_ustr_size(texto);
    if (al_ustr_prev(texto, &pos)){
        al_ustr_truncate(texto, pos);
    }
}

void campo_digitar(ALLEGRO_USTR *texto, int letra, ALLEGRO_FONT *fonte)
{
    if (letra < 32 || letra >= 127){
        return;
    }

    al_ustr_append_chr(texto, letra);

    int largura_max = (CAMPO_X2 - CAMPO_X1) - 2 * CAMPO_PADDING - al_get_text_width(fonte, CAMPO_PREFIXO);
    if (al_get_ustr_width(fonte, texto) > largura_max){
        campo_apagar_ultima_letra(texto);
    }
}

void campo_desenhar(const ALLEGRO_USTR *texto, ALLEGRO_FONT *fonte)
{
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR cinza = al_map_rgb(150, 150, 150);
    int linha = al_get_font_line_height(fonte);

    al_draw_rectangle(CAMPO_X1, CAMPO_Y1, CAMPO_X2, CAMPO_Y2, branco, 2);

    int texto_x = CAMPO_X1 + CAMPO_PADDING;
    int texto_y = CAMPO_Y1 + ((CAMPO_Y2 - CAMPO_Y1) - linha) / 2;

    al_draw_text(fonte, cinza, texto_x, texto_y, ALLEGRO_ALIGN_LEFT, CAMPO_PREFIXO);
    texto_x += al_get_text_width(fonte, CAMPO_PREFIXO);
    al_draw_ustr(fonte, branco, texto_x, texto_y, ALLEGRO_ALIGN_LEFT, texto);

    if (((int)(al_get_time() * 2)) % 2 == 0){
        int cursor_x = texto_x + al_get_ustr_width(fonte, texto) + 2;
        al_draw_line(cursor_x, texto_y, cursor_x, texto_y + linha, branco, 2);
    }
}
