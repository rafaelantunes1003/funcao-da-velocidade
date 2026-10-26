#include <stdio.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

int main(){

    al_init();
    al_init_primitives_addon();

    ALLEGRO_DISPLAY *janela = al_create_display(800, 600);

    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    al_register_event_source(fila, al_get_display_event_source(janela));

    float x = 50;
    float y = 300;
    float velocidade = 3;
    bool rodando = true;

    while (rodando){

        ALLEGRO_EVENT evento;
        while (al_get_next_event(fila, &evento)){
            if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
                rodando = false;
            }
        }

        x = x + velocidade;

        if (x >= 750){
            velocidade = -3;
        }

        if (x <= 50){
            velocidade = 3;
        }

        al_clear_to_color(al_map_rgb(0, 0, 0));

        al_draw_filled_circle(
            x,
            y,
            30,
            al_map_rgb(255, 0, 0)
        );

        al_flip_display();

        al_rest(0.01);
    }

    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    return 0;
}
