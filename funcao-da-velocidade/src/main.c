#include <stdio.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "config.h"
#include "jogo.h"
#include "desenho.h"
#include "campo_texto.h"

static bool iniciar_allegro(void)
{
    if (!al_init()){
        fprintf(stderr, "Falha ao inicializar o Allegro.\n");
        return false;
    }
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    if (!al_install_keyboard()){
        fprintf(stderr, "Falha ao inicializar o teclado.\n");
        return false;
    }
    return true;
}

static ALLEGRO_FONT *carregar_fonte(void)
{
    ALLEGRO_FONT *fonte = al_load_ttf_font("arial.ttf", 18, 0);
    if (!fonte){
        fonte = al_load_ttf_font("C:/Windows/Fonts/arial.ttf", 18, 0);
    }
    if (!fonte){
        fonte = al_create_builtin_font();
    }
    return fonte;
}

static void tratar_tecla(Jogo *jogo, ALLEGRO_USTR *texto, ALLEGRO_FONT *fonte, const ALLEGRO_KEYBOARD_EVENT *tecla)
{
    if (tecla->keycode == ALLEGRO_KEY_ENTER || tecla->keycode == ALLEGRO_KEY_PAD_ENTER){
        jogo_largar(jogo, al_cstr(texto));
    }
    else if (tecla->keycode == ALLEGRO_KEY_BACKSPACE){
        campo_apagar_ultima_letra(texto);
    }
    else {
        campo_digitar(texto, tecla->unichar, fonte);
    }
}

int main(){

    if (!iniciar_allegro()){
        return -1;
    }

    ALLEGRO_DISPLAY *janela = al_create_display(LARGURA, ALTURA);
    if (!janela){
        fprintf(stderr, "Falha ao criar a janela.\n");
        return -1;
    }
    al_set_window_title(janela, "Funcao da Velocidade");

    ALLEGRO_BITMAP *carro = al_load_bitmap(CARRO_IMAGEM);
    if (!carro){
        fprintf(stderr, "Falha ao carregar %s (confira se a pasta images esta junto do executavel).\n", CARRO_IMAGEM);
        al_destroy_display(janela);
        return -1;
    }

    ALLEGRO_FONT *fonte = carregar_fonte();

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    ALLEGRO_USTR *texto = al_ustr_new("");
    Jogo jogo;
    jogo_iniciar(&jogo);

    bool rodando = true;
    bool redesenhar = true;

    al_start_timer(timer);

    while (rodando){

        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        switch (evento.type){
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            rodando = false;
            break;

        case ALLEGRO_EVENT_KEY_CHAR:
            tratar_tecla(&jogo, texto, fonte, &evento.keyboard);
            break;

        case ALLEGRO_EVENT_TIMER:
            jogo_atualizar(&jogo);
            redesenhar = true;
            break;
        }

        if (redesenhar && al_is_event_queue_empty(fila)){
            redesenhar = false;
            desenhar_tela(&jogo, texto, carro, fonte);
        }
    }

    jogo_finalizar(&jogo);
    al_ustr_free(texto);
    al_destroy_font(fonte);
    al_destroy_bitmap(carro);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    return 0;
}
