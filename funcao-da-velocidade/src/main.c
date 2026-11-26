#include <stdio.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "config.h"
#include "menu.h"
#include "jogo.h"
#include "desenho.h"
#include "campo_texto.h"
#include "obstaculos.h"
#include "fases.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* Se a pasta images não estiver na pasta atual (por exemplo, quando o jogo
 * é aberto por um atalho), procura ao lado do próprio executável. */
static void achar_pasta_images(void)
{
    if (al_filename_exists("images/capa.png")) {
        return;
    }
    ALLEGRO_PATH* pasta = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    if (pasta) {
        al_change_directory(al_path_cstr(pasta, ALLEGRO_NATIVE_PATH_SEP));
        al_destroy_path(pasta);
    }
}

/* Mostra o erro numa janela, porque o executável pronto não abre o console */
static void mostrar_erro(const char* mensagem)
{
    fprintf(stderr, "%s\n", mensagem);
#ifdef _WIN32
    MessageBoxA(NULL, mensagem, "Funcao da Velocidade", MB_OK | MB_ICONERROR);
#endif
}

static bool iniciar_allegro(void)
{
    if (!al_init()) {
        fprintf(stderr, "Falha ao inicializar o Allegro.\n");
        return false;
    }
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    if (!al_install_keyboard()) {
        fprintf(stderr, "Falha ao inicializar o teclado.\n");
        return false;
    }
    if (!al_install_mouse()) {
        fprintf(stderr, "Falha ao inicializar o mouse.\n");
        return false;
    }
    return true;
}

static ALLEGRO_FONT* carregar_fonte(void)
{
    ALLEGRO_FONT* fonte = al_load_ttf_font("arial.ttf", 18, 0);
    if (!fonte) {
        fonte = al_load_ttf_font("C:/Windows/Fonts/arial.ttf", 18, 0);
    }
    if (!fonte) {
        fonte = al_create_builtin_font();
    }
    return fonte;
}

static void tratar_tecla(Jogo* jogo, ALLEGRO_USTR* texto, ALLEGRO_FONT* fonte, const ALLEGRO_KEYBOARD_EVENT* tecla)
{
    if (tecla->keycode == ALLEGRO_KEY_ENTER || tecla->keycode == ALLEGRO_KEY_PAD_ENTER) {
        if (jogo->estado == CHEGOU && jogo_tem_proxima_fase(jogo)) {
            jogo_proxima_fase(jogo);
            al_ustr_truncate(texto, 0);
        }
        else {
            jogo_largar(jogo, al_cstr(texto));
        }
    }
    else if (tecla->keycode == ALLEGRO_KEY_BACKSPACE) {
        campo_apagar_ultima_letra(texto);
    }
    else {
        campo_digitar(texto, tecla->unichar, fonte);
    }
}

int main(void)
{
    if (!iniciar_allegro()) {
        return -1;
    }
    achar_pasta_images();

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA, ALTURA);
    if (!janela) {
        fprintf(stderr, "Falha ao criar a janela.\n");
        return -1;
    }
    al_set_window_title(janela, "Funcao da Velocidade");

    Menu menu;
    if (!menu_inicializar(&menu, "images/capa.png")) {
        mostrar_erro("Nao encontrei a pasta images.\n"
                     "Deixe o executavel do jogo na mesma pasta da pasta images e das DLLs.");
        al_destroy_display(janela);
        return -1;
    }

    ALLEGRO_BITMAP* carro = al_load_bitmap(CARRO_IMAGEM);
    if (!carro) {
        fprintf(stderr, "Falha ao carregar %s (confira se a pasta images esta junto do executavel).\n", CARRO_IMAGEM);
        menu_destruir(&menu);
        al_destroy_display(janela);
        return -1;
    }

    if (!obstaculos_inicializar()) {
        fprintf(stderr, "Falha ao carregar as imagens dos obstaculos (confira a pasta images).\n");
        al_destroy_bitmap(carro);
        menu_destruir(&menu);
        al_destroy_display(janela);
        return -1;
    }

    if (!fases_inicializar()) {
        obstaculos_finalizar();
        al_destroy_bitmap(carro);
        menu_destruir(&menu);
        al_destroy_display(janela);
        return -1;
    }

    ALLEGRO_FONT* fonte = carregar_fonte();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_mouse_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    ALLEGRO_USTR* texto = al_ustr_new("");
    Jogo jogo;
    jogo_iniciar(&jogo);

    Tela tela_atual = TELA_INICIAL;
    bool rodando = true;
    bool redesenhar = true;

    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        switch (evento.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            rodando = false;
            break;

        case ALLEGRO_EVENT_MOUSE_AXES:
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (tela_atual == TELA_INICIAL) {
                if (menu_processar_evento(&menu, &evento)) {
                    tela_atual = TELA_SELECAO_FASES;
                }
                redesenhar = true;
            }
            else if (tela_atual == TELA_SELECAO_FASES) {
                if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
                    && evento.mouse.button == 1) {
                    if (desenho_clicou_voltar_fases(evento.mouse.x, evento.mouse.y)) {
                        tela_atual = TELA_INICIAL;
                    } else {
                        int fase = desenho_identificar_fase(evento.mouse.x, evento.mouse.y);
                        if (fase >= 0) {
                            jogo.fase = fase;
                            jogo_reiniciar_fase(&jogo);
                            al_ustr_truncate(texto, 0);
                            tela_atual = TELA_JOGO;
                        }
                    }
                }
                redesenhar = true;
            }
            else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
                && evento.mouse.button == 1) {
                int botao = desenho_identificar_botao(
                    evento.mouse.x, evento.mouse.y, &jogo);

                if (botao == BOTAO_MENU) {
                    jogo_finalizar(&jogo);
                    jogo_iniciar(&jogo);
                    al_ustr_truncate(texto, 0);
                    tela_atual = TELA_INICIAL;
                }
                else if (botao == BOTAO_SELECIONAR_FASES) {
                    tela_atual = TELA_SELECAO_FASES;
                }
                else if (botao == BOTAO_REINICIAR) {
                    jogo_reiniciar_fase(&jogo);
                    al_ustr_truncate(texto, 0);
                }
                else if (botao == BOTAO_PROXIMA_FASE) {
                    jogo_proxima_fase(&jogo);
                    al_ustr_truncate(texto, 0);
                }
                redesenhar = true;
            }
            break;

        case ALLEGRO_EVENT_KEY_CHAR:
            if (tela_atual == TELA_JOGO) {
                tratar_tecla(&jogo, texto, fonte, &evento.keyboard);
                redesenhar = true;
            }
            break;

        case ALLEGRO_EVENT_TIMER:
            if (tela_atual == TELA_JOGO) {
                jogo_atualizar(&jogo);
            }
            redesenhar = true;
            break;
        }

        if (redesenhar && al_is_event_queue_empty(fila)) {
            redesenhar = false;
            if (tela_atual == TELA_INICIAL) {
                menu_desenhar(&menu, fonte, LARGURA, ALTURA);
            }
            else if (tela_atual == TELA_SELECAO_FASES) {
                desenho_tela_fases(jogo.fase, fonte);
            }
            else {
                desenhar_tela(&jogo, texto, carro, fonte);
            }
        }
    }

    jogo_finalizar(&jogo);
    al_ustr_free(texto);
    al_destroy_font(fonte);
    obstaculos_finalizar();
    fases_finalizar();
    al_destroy_bitmap(carro);
    menu_destruir(&menu);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    return 0;
}