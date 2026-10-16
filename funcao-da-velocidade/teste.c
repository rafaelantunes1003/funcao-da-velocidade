#include <stdio.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define LARGURA 800
#define ALTURA  600
#define FPS     60.0

/* Retângulo do botão */
#define BTN_LARGURA 240
#define BTN_ALTURA  70
#define BTN_X1 ((LARGURA - BTN_LARGURA) / 2)
#define BTN_Y1 (ALTURA - 180)
#define BTN_X2 (BTN_X1 + BTN_LARGURA)
#define BTN_Y2 (BTN_Y1 + BTN_ALTURA)

/* Verifica se o ponto (x, y) está dentro do botão */
static bool dentro_do_botao(int x, int y)
{
    return x >= BTN_X1 && x <= BTN_X2 && y >= BTN_Y1 && y <= BTN_Y2;
}

int main(void)
{
    if (!al_init()) {
        fprintf(stderr, "Falha ao inicializar o Allegro.\n");
        return -1;
    }
    if (!al_init_image_addon()) {
        fprintf(stderr, "Falha ao inicializar o addon de imagens.\n");
        return -1;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Falha ao inicializar o addon de primitivas.\n");
        return -1;
    }
    al_init_font_addon();
    al_init_ttf_addon();

    if (!al_install_mouse()) {
        fprintf(stderr, "Falha ao inicializar o mouse.\n");
        return -1;
    }

    ALLEGRO_DISPLAY* display = al_create_display(LARGURA, ALTURA);
    if (!display) {
        fprintf(stderr, "Falha ao criar a janela.\n");
        return -1;
    }
    al_set_window_title(display, "Meu Jogo");

    ALLEGRO_BITMAP* capa = al_load_bitmap("capa.png");
    if (!capa) {
        fprintf(stderr, "Falha ao carregar capa.png (confira se o arquivo esta na pasta do executavel).\n");
        al_destroy_display(display);
        return -1;
    }

    /* Tenta carregar uma fonte TTF; se não achar, usa a fonte embutida */
    ALLEGRO_FONT* fonte = al_load_ttf_font("arial.ttf", 36, 0);
    if (!fonte) {
        fonte = al_create_builtin_font();
    }

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();

    al_register_event_source(fila, al_get_display_event_source(display));
    al_register_event_source(fila, al_get_mouse_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    ALLEGRO_COLOR verde = al_map_rgb(0, 170, 0);
    ALLEGRO_COLOR verde_hover = al_map_rgb(40, 210, 40);
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);

    bool rodando = true;
    bool redesenhar = true;
    bool mouse_sobre_botao = false;

    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila, &ev);

        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            rodando = false;
            break;

        case ALLEGRO_EVENT_MOUSE_AXES:
            mouse_sobre_botao = dentro_do_botao(ev.mouse.x, ev.mouse.y);
            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            /* Botão esquerdo do mouse solto sobre o botão "Jogar" */
            if (ev.mouse.button == 1 && dentro_do_botao(ev.mouse.x, ev.mouse.y)) {
                printf("Botao JOGAR clicado!\n");
                /* TODO: aqui você inicia o jogo (troca de tela/estado) */
            }
            break;

        case ALLEGRO_EVENT_TIMER:
            redesenhar = true;
            break;
        }

        if (redesenhar && al_is_event_queue_empty(fila)) {
            redesenhar = false;

            /* Imagem de fundo ajustada ao tamanho da janela */
            al_draw_scaled_bitmap(capa,
                0, 0,
                al_get_bitmap_width(capa), al_get_bitmap_height(capa),
                0, 0,
                LARGURA, ALTURA, 0);

            /* Botão verde com borda branca */
            al_draw_filled_rectangle(BTN_X1, BTN_Y1, BTN_X2, BTN_Y2,
                mouse_sobre_botao ? verde_hover : verde);
            al_draw_rectangle(BTN_X1, BTN_Y1, BTN_X2, BTN_Y2, branco, 4);

            /* Texto "JOGAR" centralizado no botão */
            int texto_y = BTN_Y1 + (BTN_ALTURA - al_get_font_line_height(fonte)) / 2;
            al_draw_text(fonte, branco, (BTN_X1 + BTN_X2) / 2, texto_y,
                ALLEGRO_ALIGN_CENTRE, "JOGAR");

            al_flip_display();
        }
    }

    al_destroy_font(fonte);
    al_destroy_bitmap(capa);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(display);

    return 0;
}