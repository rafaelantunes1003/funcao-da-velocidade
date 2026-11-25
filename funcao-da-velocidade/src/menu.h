#pragma once
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

typedef enum {
    TELA_INICIAL,
    TELA_SELECAO_FASES,
    TELA_JOGO
} Tela;

typedef struct {
    ALLEGRO_BITMAP* capa;
    bool mouse_sobre_botao;
} Menu;

bool menu_inicializar(Menu* m, const char* caminho_capa);
bool menu_processar_evento(Menu* m, const ALLEGRO_EVENT* evento);
void menu_desenhar(const Menu* m, ALLEGRO_FONT* fonte, int largura_tela, int altura_tela);
void menu_destruir(Menu* m);