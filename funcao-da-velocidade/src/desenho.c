#include <math.h>
#include <stdbool.h>
#include <allegro5/allegro_primitives.h>
#include "desenho.h"
#include "config.h"
#include "coordenadas.h"
#include "campo_texto.h"
#include "obstaculos.h"
#include "fases.h"

static void desenhar_funcao(const Expressao *funcao, ALLEGRO_COLOR cor, int indice_fase)
{
    double y_min = Y_MIN;
    double y_max = Y_MAX;

    if (indice_fase == 1) {
        y_min = FASE2_Y_MIN;
        y_max = FASE2_Y_MAX;
    } else if (indice_fase == 2) {
        y_min = FASE3_Y_MIN;
        y_max = FASE3_Y_MAX;
    }
    double margem = y_max - y_min;
    float altura_pista = PISTA_Y2 - PISTA_Y1;
    bool anterior_ok = false;
    float px_anterior = 0, py_anterior = 0;
    al_set_clipping_rectangle(PISTA_X1, PISTA_Y1,
        PISTA_X2 - PISTA_X1 + 1, PISTA_Y2 - PISTA_Y1 + 1);
    for (int px = PISTA_X1; px <= PISTA_X2; px++) {
        double x = X_MIN + (px - PISTA_X1) * (X_MAX - X_MIN) / (PISTA_X2 - PISTA_X1);
        double y = expressao_avaliar(funcao, x);
        bool ok = isfinite(y) && y > y_min - margem && y < y_max + margem;
        float py = ok ? tela_y_fase(y, indice_fase) : 0;
        if (ok && anterior_ok && fabsf(py - py_anterior) < altura_pista)
            al_draw_line(px_anterior, py_anterior, (float)px, py, cor, 3);
        anterior_ok = ok;
        px_anterior = (float)px;
        py_anterior = py;
    }
    al_reset_clipping_rectangle();
}

static void desenhar_pista_parabola(const Fase *fase)
{
    ALLEGRO_COLOR grama = al_map_rgb(48, 105, 55);
    ALLEGRO_COLOR borda = al_map_rgb(190, 185, 165);
    ALLEGRO_COLOR asfalto = al_map_rgb(55, 58, 62);
    ALLEGRO_COLOR faixa = al_map_rgb(245, 205, 65);

    int fim_pista = (int)tela_x(X_CHEGADA);
    al_clear_to_color(grama);

    /* acostamento e asfalto acompanham a curva ate a chegada */
    for (int px = PISTA_X1; px < fim_pista; px += 2) {
        double x0 = X_MIN + (px - PISTA_X1) * (X_MAX - X_MIN) / (PISTA_X2 - PISTA_X1);
        double x1 = X_MIN + (px + 2 - PISTA_X1) * (X_MAX - X_MIN) / (PISTA_X2 - PISTA_X1);
        float y0 = tela_y_fase(fase->centro(x0), 2);
        float y1 = tela_y_fase(fase->centro(x1), 2);

        al_draw_line((float)px, y0, (float)(px + 2), y1, borda, 126);
        al_draw_line((float)px, y0, (float)(px + 2), y1, asfalto, 116);
    }

    /* linha central tracejada */
    for (int px = PISTA_X1; px < fim_pista; px += 42) {
        int fim = px + 20;
        if (fim > fim_pista) fim = fim_pista;
        double x0 = X_MIN + (px - PISTA_X1) * (X_MAX - X_MIN) / (PISTA_X2 - PISTA_X1);
        double x1 = X_MIN + (fim - PISTA_X1) * (X_MAX - X_MIN) / (PISTA_X2 - PISTA_X1);
        al_draw_line((float)px, tela_y_fase(fase->centro(x0), 2),
            (float)fim, tela_y_fase(fase->centro(x1), 2), faixa, 3);
    }

    /* faixa quadriculada na chegada, perpendicular a estrada */
    {
        double x = X_CHEGADA;
        double antes = fase->centro(x - 0.01);
        double depois = fase->centro(x + 0.01);
        float cx = tela_x(x);
        float cy = tela_y_fase(fase->centro(x), 2);
        float dx = tela_x(x + 0.01) - tela_x(x - 0.01);
        float dy = tela_y_fase(depois, 2) - tela_y_fase(antes, 2);
        float tamanho = sqrtf(dx * dx + dy * dy);
        float nx = -dy / tamanho;
        float ny = dx / tamanho;

        for (int i = 0; i < 8; i++) {
            float a = -56.0f + i * 14.0f;
            float b = a + 14.0f;
            ALLEGRO_COLOR cor = (i % 2 == 0)
                ? al_map_rgb(245, 245, 245) : al_map_rgb(25, 25, 25);
            al_draw_line(cx + nx * a, cy + ny * a,
                cx + nx * b, cy + ny * b, cor, 10);
        }
    }
}

static void desenhar_carro(const Jogo *jogo, ALLEGRO_BITMAP *carro)
{
    al_draw_scaled_rotated_bitmap(carro, CARRO_CENTRO_X, CARRO_CENTRO_Y,
        tela_x(jogo->x), tela_y_fase(jogo->y, jogo->fase), CARRO_ESCALA, CARRO_ESCALA,
        jogo->angulo, 0);
}

static void desenhar_vitoria(const Jogo *jogo, ALLEGRO_FONT *fonte)
{
    ALLEGRO_COLOR fundo = al_map_rgba(10, 35, 18, 235);
    ALLEGRO_COLOR verde = al_map_rgb(100, 255, 120);
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);

    al_draw_filled_rectangle(150, 190, 670, 300, fundo);
    al_draw_rectangle(150, 190, 670, 300, verde, 4);
    al_draw_text(fonte, verde, LARGURA / 2, 210, ALLEGRO_ALIGN_CENTER,
        "VITORIA!");

    if (jogo_tem_proxima_fase(jogo)) {
        al_draw_textf(fonte, branco, LARGURA / 2, 245, ALLEGRO_ALIGN_CENTER,
            "Voce passou da fase %d!", jogo->fase + 1);
        al_draw_textf(fonte, branco, LARGURA / 2, 270, ALLEGRO_ALIGN_CENTER,
            "Aperte ENTER para ir para a fase %d.", jogo->fase + 2);
    } else {
        al_draw_text(fonte, branco, LARGURA / 2, 245, ALLEGRO_ALIGN_CENTER,
            "Voce completou todas as fases!");
        al_draw_text(fonte, branco, LARGURA / 2, 270, ALLEGRO_ALIGN_CENTER,
            "Digite outra funcao e aperte ENTER para jogar novamente.");
    }
}

static void desenhar_numero_da_fase(const Jogo *jogo, ALLEGRO_FONT *fonte)
{
    int altura = al_get_font_line_height(fonte);

    al_draw_filled_rectangle(PISTA_X1, PISTA_Y1, PISTA_X1 + 90, PISTA_Y1 + altura + 10,
        al_map_rgba(0, 0, 0, 180));
    al_draw_textf(fonte, al_map_rgb(255, 255, 255), PISTA_X1 + 45, PISTA_Y1 + 5,
        ALLEGRO_ALIGN_CENTER, "FASE %d", jogo->fase + 1);
}

static void desenhar_mensagem(const Jogo *jogo, ALLEGRO_FONT *fonte)
{
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR cinza = al_map_rgb(150, 150, 150);
    ALLEGRO_COLOR verde = al_map_rgb(60, 220, 60);
    ALLEGRO_COLOR vermelho = al_map_rgb(255, 80, 80);
    int x = CAMPO_X1;
    int y = PISTA_Y2 + al_get_font_line_height(fonte) + 14;

    if (!jogo->funcao && jogo->erro[0] != '\0') {
        al_draw_textf(fonte, vermelho, x, y, ALLEGRO_ALIGN_LEFT, "Erro: %s", jogo->erro);
    } else if (jogo->estado == PARADO) {
        al_draw_text(fonte, cinza, x, y, ALLEGRO_ALIGN_LEFT, fases_obter(jogo->fase)->dica);
    } else if (jogo->estado == ANDANDO) {
        al_draw_textf(fonte, branco, x, y, ALLEGRO_ALIGN_LEFT,
            "x = %.2f    f(x) = %.2f", jogo->x, jogo->y);
    } else if (jogo->estado == CHEGOU && jogo_tem_proxima_fase(jogo)) {
        al_draw_textf(fonte, verde, x, y, ALLEGRO_ALIGN_LEFT,
            "Chegou! f(%.2f) = %.2f   (ENTER vai para a fase %d)", jogo->x, jogo->y, jogo->fase + 2);
    } else if (jogo->estado == CHEGOU) {
        al_draw_textf(fonte, verde, x, y, ALLEGRO_ALIGN_LEFT,
            "Chegou! f(%.2f) = %.2f   (ENTER corre de novo)", jogo->x, jogo->y);
    } else if (jogo->estado == COLIDIU) {
        const char *nome = "obstaculo";
        if (jogo->obstaculo_colisao) {
            switch (jogo->obstaculo_colisao->tipo) {
            case OBSTACULO_PEDRA: nome = "pedra"; break;
            case OBSTACULO_CONE: nome = "cone"; break;
            case OBSTACULO_BURACO: nome = "buraco"; break;
            }
        }
        al_draw_textf(fonte, vermelho, x, y, ALLEGRO_ALIGN_LEFT,
            "Colisao com %s em x = %.2f! (ENTER tenta novamente)", nome, jogo->x);
    } else if (!isfinite(jogo->y_saida)) {
        al_draw_textf(fonte, vermelho, x, y, ALLEGRO_ALIGN_LEFT,
            "f(x) nao existe em x = %.2f   (ENTER corre de novo)", jogo->x_saida);
    } else {
        al_draw_textf(fonte, vermelho, x, y, ALLEGRO_ALIGN_LEFT,
            "Saiu da pista em x = %.2f: f(x) = %.2f   (ENTER corre de novo)",
            jogo->x_saida, jogo->y_saida);
    }
}

void desenhar_tela(const Jogo *jogo, const ALLEGRO_USTR *texto,
                   ALLEGRO_BITMAP *carro, ALLEGRO_FONT *fonte)
{
    const Fase *fase = fases_obter(jogo->fase);
    ALLEGRO_BITMAP *pista = fases_imagem(jogo->fase);

    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (jogo->fase == 2) {
        desenhar_pista_parabola(fase);
    } else if (pista) {
        al_draw_scaled_bitmap(pista, 0, 0,
            (float)al_get_bitmap_width(pista), (float)al_get_bitmap_height(pista),
            PISTA_X1, PISTA_Y1, PISTA_X2 - PISTA_X1, PISTA_Y2 - PISTA_Y1, 0);
    }
    if (jogo->funcao) desenhar_funcao(jogo->funcao, al_map_rgb(255, 200, 0), jogo->fase);
    obstaculos_desenhar(fase->obstaculos, fase->num_obstaculos, jogo->fase);
    desenhar_carro(jogo, carro);
    desenhar_numero_da_fase(jogo, fonte);
    if (jogo->estado == CHEGOU) desenhar_vitoria(jogo, fonte);
    desenhar_mensagem(jogo, fonte);
    campo_desenhar(texto, fonte);
    al_flip_display();
}
