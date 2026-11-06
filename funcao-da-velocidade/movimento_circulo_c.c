#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "InterpretaFuncao/expressao.h"

#define LARGURA 800
#define ALTURA  600
#define FPS     60.0

#define PISTA_X1 50
#define PISTA_Y1 30
#define PISTA_X2 770
#define PISTA_Y2 450

#define X_MIN 0.0
#define X_MAX 10.0
#define Y_MIN -5.0
#define Y_MAX 5.0

#define CAMPO_X1 50
#define CAMPO_Y1 520
#define CAMPO_X2 770
#define CAMPO_Y2 565
#define CAMPO_PADDING 10

#define CARRO_CENTRO_X 334
#define CARRO_CENTRO_Y 191
#define CARRO_ESCALA   0.22f
#define CARRO_DIREITA  ((float)(ALLEGRO_PI / 2))

#define VELOCIDADE 1.5

typedef enum {
    PARADO,
    ANDANDO,
    CHEGOU,
    SAIU
} Estado;

static float tela_x(double x)
{
    return (float)(PISTA_X1 + (x - X_MIN) * (PISTA_X2 - PISTA_X1) / (X_MAX - X_MIN));
}

static float tela_y(double y)
{
    return (float)(PISTA_Y2 - (y - Y_MIN) * (PISTA_Y2 - PISTA_Y1) / (Y_MAX - Y_MIN));
}

static bool dentro_da_pista(double y)
{
    return isfinite(y) && y >= Y_MIN && y <= Y_MAX;
}

static float angulo_na_curva(const Expressao *funcao, double x, float angulo_atual)
{
    double passo = 0.01;
    double y1 = expressao_avaliar(funcao, x);
    double y2 = expressao_avaliar(funcao, x + passo);

    if (!isfinite(y1) || !isfinite(y2)){
        return angulo_atual;
    }

    float dx = tela_x(x + passo) - tela_x(x);
    float dy = tela_y(y2) - tela_y(y1);

    return atan2f(dy, dx) + CARRO_DIREITA;
}

static void desenhar_grade(ALLEGRO_FONT *fonte)
{
    ALLEGRO_COLOR grade = al_map_rgb(45, 45, 45);
    ALLEGRO_COLOR eixo = al_map_rgb(140, 140, 140);
    ALLEGRO_COLOR numero = al_map_rgb(120, 120, 120);
    int altura_fonte = al_get_font_line_height(fonte);

    for (int i = (int)X_MIN; i <= (int)X_MAX; i++){
        float px = tela_x(i);
        al_draw_line(px, PISTA_Y1, px, PISTA_Y2, grade, 1);
        al_draw_textf(fonte, numero, px, PISTA_Y2 + 6, ALLEGRO_ALIGN_CENTRE, "%d", i);
    }

    for (int i = (int)Y_MIN; i <= (int)Y_MAX; i++){
        float py = tela_y(i);
        al_draw_line(PISTA_X1, py, PISTA_X2, py, grade, 1);
        al_draw_textf(fonte, numero, PISTA_X1 - 8, py - altura_fonte / 2.0f, ALLEGRO_ALIGN_RIGHT, "%d", i);
    }

    al_draw_line(PISTA_X1, tela_y(0), PISTA_X2, tela_y(0), eixo, 2);
    al_draw_line(tela_x(0), PISTA_Y1, tela_x(0), PISTA_Y2, eixo, 2);
    al_draw_rectangle(PISTA_X1, PISTA_Y1, PISTA_X2, PISTA_Y2, eixo, 2);
}

static void desenhar_funcao(const Expressao *funcao, ALLEGRO_COLOR cor)
{
    double margem = Y_MAX - Y_MIN;
    float altura_pista = PISTA_Y2 - PISTA_Y1;
    bool anterior_ok = false;
    float px_anterior = 0;
    float py_anterior = 0;

    al_set_clipping_rectangle(PISTA_X1, PISTA_Y1, PISTA_X2 - PISTA_X1 + 1, PISTA_Y2 - PISTA_Y1 + 1);

    for (int px = PISTA_X1; px <= PISTA_X2; px++){
        double x = X_MIN + (px - PISTA_X1) * (X_MAX - X_MIN) / (PISTA_X2 - PISTA_X1);
        double y = expressao_avaliar(funcao, x);
        bool ok = isfinite(y) && y > Y_MIN - margem && y < Y_MAX + margem;
        float py = ok ? tela_y(y) : 0;

        if (ok && anterior_ok && fabsf(py - py_anterior) < altura_pista){
            al_draw_line(px_anterior, py_anterior, (float)px, py, cor, 3);
        }

        anterior_ok = ok;
        px_anterior = (float)px;
        py_anterior = py;
    }

    al_reset_clipping_rectangle();
}

int main(){

    if (!al_init()){
        fprintf(stderr, "Falha ao inicializar o Allegro.\n");
        return -1;
    }
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    if (!al_install_keyboard()){
        fprintf(stderr, "Falha ao inicializar o teclado.\n");
        return -1;
    }

    ALLEGRO_DISPLAY *janela = al_create_display(LARGURA, ALTURA);
    if (!janela){
        fprintf(stderr, "Falha ao criar a janela.\n");
        return -1;
    }
    al_set_window_title(janela, "Funcao da Velocidade");

    ALLEGRO_BITMAP *carro = al_load_bitmap("images/image-removebg-preview.png");
    if (!carro){
        fprintf(stderr, "Falha ao carregar images/image-removebg-preview.png (confira se a pasta images esta junto do executavel).\n");
        al_destroy_display(janela);
        return -1;
    }

    ALLEGRO_FONT *fonte = al_load_ttf_font("arial.ttf", 18, 0);
    if (!fonte){
        fonte = al_load_ttf_font("C:/Windows/Fonts/arial.ttf", 18, 0);
    }
    if (!fonte){
        fonte = al_create_builtin_font();
    }

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR cinza = al_map_rgb(150, 150, 150);
    ALLEGRO_COLOR amarelo = al_map_rgb(255, 200, 0);
    ALLEGRO_COLOR verde = al_map_rgb(60, 220, 60);
    ALLEGRO_COLOR vermelho = al_map_rgb(255, 80, 80);

    ALLEGRO_USTR *texto = al_ustr_new("");
    Expressao *funcao = NULL;
    char erro[128] = "";

    Estado estado = PARADO;
    int passos = 0;
    double x = X_MIN;
    double y = 0;
    double x_saida = 0;
    double y_saida = 0;
    float angulo = CARRO_DIREITA;

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
            if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER){
                Expressao *nova = expressao_compilar(al_cstr(texto), erro, sizeof(erro));

                expressao_liberar(funcao);
                funcao = nova;
                passos = 0;
                x = X_MIN;
                y = 0;
                angulo = CARRO_DIREITA;

                if (!funcao){
                    estado = PARADO;
                }
                else {
                    double y_inicio = expressao_avaliar(funcao, x);

                    if (dentro_da_pista(y_inicio)){
                        y = y_inicio;
                        angulo = angulo_na_curva(funcao, x, angulo);
                        estado = ANDANDO;
                    }
                    else {
                        x_saida = x;
                        y_saida = y_inicio;
                        estado = SAIU;
                    }
                }
            }
            else if (evento.keyboard.keycode == ALLEGRO_KEY_BACKSPACE){
                int pos = (int)al_ustr_size(texto);
                if (al_ustr_prev(texto, &pos)){
                    al_ustr_truncate(texto, pos);
                }
            }
            else if (evento.keyboard.unichar >= 32 && evento.keyboard.unichar < 127){
                al_ustr_append_chr(texto, evento.keyboard.unichar);

                int largura_max = (CAMPO_X2 - CAMPO_X1) - 2 * CAMPO_PADDING - al_get_text_width(fonte, "f(x) = ");
                if (al_get_ustr_width(fonte, texto) > largura_max){
                    int pos = (int)al_ustr_size(texto);
                    if (al_ustr_prev(texto, &pos)){
                        al_ustr_truncate(texto, pos);
                    }
                }
            }
            break;

        case ALLEGRO_EVENT_TIMER:
            if (estado == ANDANDO){
                double novo_x = X_MIN + (passos + 1) * VELOCIDADE / FPS;
                if (novo_x > X_MAX){
                    novo_x = X_MAX;
                }

                double novo_y = expressao_avaliar(funcao, novo_x);

                if (!dentro_da_pista(novo_y)){
                    x_saida = novo_x;
                    y_saida = novo_y;
                    estado = SAIU;
                }
                else {
                    passos++;
                    x = novo_x;
                    y = novo_y;
                    angulo = angulo_na_curva(funcao, x, angulo);

                    if (x >= X_MAX){
                        estado = CHEGOU;
                    }
                }
            }
            redesenhar = true;
            break;
        }

        if (redesenhar && al_is_event_queue_empty(fila)){
            redesenhar = false;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            desenhar_grade(fonte);

            if (funcao){
                desenhar_funcao(funcao, amarelo);
            }

            al_draw_scaled_rotated_bitmap(
                carro,
                CARRO_CENTRO_X,
                CARRO_CENTRO_Y,
                tela_x(x),
                tela_y(y),
                CARRO_ESCALA,
                CARRO_ESCALA,
                angulo,
                0
            );

            int linha = al_get_font_line_height(fonte);
            int mensagem_y = PISTA_Y2 + linha + 14;

            if (!funcao && erro[0] != '\0'){
                al_draw_textf(fonte, vermelho, CAMPO_X1, mensagem_y, ALLEGRO_ALIGN_LEFT, "Erro: %s", erro);
            }
            else if (estado == PARADO){
                al_draw_text(fonte, cinza, CAMPO_X1, mensagem_y, ALLEGRO_ALIGN_LEFT,
                    "Digite uma funcao de x e aperte ENTER. Exemplos: sen(x)   x/2   2sen(x)+1");
            }
            else if (estado == ANDANDO){
                al_draw_textf(fonte, branco, CAMPO_X1, mensagem_y, ALLEGRO_ALIGN_LEFT,
                    "x = %.2f    f(x) = %.2f", x, y);
            }
            else if (estado == CHEGOU){
                al_draw_textf(fonte, verde, CAMPO_X1, mensagem_y, ALLEGRO_ALIGN_LEFT,
                    "Chegou! f(%.0f) = %.2f   (ENTER corre de novo)", x, y);
            }
            else if (!isfinite(y_saida)){
                al_draw_textf(fonte, vermelho, CAMPO_X1, mensagem_y, ALLEGRO_ALIGN_LEFT,
                    "f(x) nao existe em x = %.2f   (ENTER corre de novo)", x_saida);
            }
            else {
                al_draw_textf(fonte, vermelho, CAMPO_X1, mensagem_y, ALLEGRO_ALIGN_LEFT,
                    "Saiu da pista em x = %.2f: f(x) = %.2f   (ENTER corre de novo)", x_saida, y_saida);
            }

            al_draw_rectangle(CAMPO_X1, CAMPO_Y1, CAMPO_X2, CAMPO_Y2, branco, 2);

            int texto_x = CAMPO_X1 + CAMPO_PADDING;
            int texto_y = CAMPO_Y1 + ((CAMPO_Y2 - CAMPO_Y1) - linha) / 2;

            al_draw_text(fonte, cinza, texto_x, texto_y, ALLEGRO_ALIGN_LEFT, "f(x) = ");
            texto_x += al_get_text_width(fonte, "f(x) = ");
            al_draw_ustr(fonte, branco, texto_x, texto_y, ALLEGRO_ALIGN_LEFT, texto);

            if (((int)(al_get_time() * 2)) % 2 == 0){
                int cursor_x = texto_x + al_get_ustr_width(fonte, texto) + 2;
                al_draw_line(cursor_x, texto_y, cursor_x, texto_y + linha, branco, 2);
            }

            al_flip_display();
        }
    }

    expressao_liberar(funcao);
    al_ustr_free(texto);
    al_destroy_font(fonte);
    al_destroy_bitmap(carro);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    return 0;
}
