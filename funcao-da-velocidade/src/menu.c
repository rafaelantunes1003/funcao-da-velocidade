#include <allegro5/allegro_primitives.h>
#include "menu.h"

#define BTN_X1 285
#define BTN_Y1 425
#define BTN_X2 515
#define BTN_Y2 492

static bool dentro_do_botao(int x, int y)
{
    return x >= BTN_X1 && x <= BTN_X2 && y >= BTN_Y1 && y <= BTN_Y2;
}

bool menu_inicializar(Menu* m, const char* caminho_capa)
{
    m->capa = al_load_bitmap(caminho_capa);
    if (!m->capa) {
        return false;
    }

    m->mouse_sobre_botao = false;
    return true;
}

bool menu_processar_evento(Menu* m, const ALLEGRO_EVENT* evento)
{
    if (evento->type == ALLEGRO_EVENT_MOUSE_AXES) {
        m->mouse_sobre_botao = dentro_do_botao(evento->mouse.x, evento->mouse.y);
    }
    else if (evento->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && evento->mouse.button == 1) {
        if (dentro_do_botao(evento->mouse.x, evento->mouse.y)) {
            return true;
        }
    }

    return false;
}

void menu_desenhar(const Menu* m, ALLEGRO_FONT* fonte, int largura_tela, int altura_tela)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_draw_scaled_bitmap(
        m->capa,
        0, 0,
        al_get_bitmap_width(m->capa),
        al_get_bitmap_height(m->capa),
        0, 0,
        largura_tela, altura_tela,
        0
    );

    /* A arte da capa já contém o logotipo e o slogan.
       Não desenhar textos por cima para manter a identidade visual legível. */

    ALLEGRO_COLOR cor_fundo = m->mouse_sobre_botao
        ? al_map_rgb(55, 205, 145) : al_map_rgb(30, 145, 110);
    ALLEGRO_COLOR cor_borda = al_map_rgb(220, 255, 245);

    al_draw_filled_rounded_rectangle(BTN_X1 + 3, BTN_Y1 + 5,
        BTN_X2 + 3, BTN_Y2 + 5, 16, 16, al_map_rgba(0, 0, 0, 125));
    al_draw_filled_rounded_rectangle(BTN_X1, BTN_Y1, BTN_X2, BTN_Y2,
        16, 16, cor_fundo);
    al_draw_rounded_rectangle(BTN_X1, BTN_Y1, BTN_X2, BTN_Y2,
        16, 16, cor_borda, 2);

    int texto_x = (BTN_X1 + BTN_X2) / 2;
    int texto_y = (BTN_Y1 + BTN_Y2 - al_get_font_line_height(fonte)) / 2;
    al_draw_text(fonte, cor_borda, texto_x, texto_y, ALLEGRO_ALIGN_CENTRE,
        "ESCOLHER FASE");
    al_flip_display();
}

void menu_destruir(Menu* m)
{
    if (m->capa) {
        al_destroy_bitmap(m->capa);
        m->capa = NULL;
    }
}