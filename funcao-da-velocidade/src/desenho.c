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
    double y_min = indice_fase == 1 ? FASE2_Y_MIN : Y_MIN;
    double y_max = indice_fase == 1 ? FASE2_Y_MAX : Y_MAX;
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
    al_draw_text(fonte, verde, LARGURA / 2, 210, ALLEGRO_ALIGN_CENTRE,
        "VITORIA!");

    if (jogo_tem_proxima_fase(jogo)) {
        al_draw_textf(fonte, branco, LARGURA / 2, 245, ALLEGRO_ALIGN_CENTRE,
            "Voce passou da fase %d!", jogo->fase + 1);
        al_draw_textf(fonte, branco, LARGURA / 2, 270, ALLEGRO_ALIGN_CENTRE,
            "Aperte ENTER para ir para a fase %d.", jogo->fase + 2);
    } else {
        al_draw_text(fonte, branco, LARGURA / 2, 245, ALLEGRO_ALIGN_CENTRE,
            "Voce completou todas as fases!");
        al_draw_text(fonte, branco, LARGURA / 2, 270, ALLEGRO_ALIGN_CENTRE,
            "Digite outra funcao e aperte ENTER para jogar novamente.");
    }
}

static void desenhar_botao(ALLEGRO_FONT *fonte, int x1, int x2,
    const char *rotulo, ALLEGRO_COLOR fundo)
{
    ALLEGRO_COLOR borda = al_map_rgb(105, 190, 230);
    int y1 = 7;
    int y2 = 31;
    al_draw_filled_rounded_rectangle((float)x1 + 2, (float)y1 + 2,
        (float)x2 + 2, (float)y2 + 2, 8, 8, al_map_rgba(0, 0, 0, 90));
    al_draw_filled_rounded_rectangle((float)x1, (float)y1, (float)x2,
        (float)y2, 8, 8, fundo);
    al_draw_rounded_rectangle((float)x1, (float)y1, (float)x2,
        (float)y2, 8, 8, borda, 1);
    al_draw_text(fonte, al_map_rgb(255, 255, 255),
        (x1 + x2) / 2, y1 + 5, ALLEGRO_ALIGN_CENTRE, rotulo);
}

int desenho_identificar_botao(int x, int y, const Jogo *jogo)
{
    if (y < 7 || y > 31) return 0;
    if (x >= 250 && x <= 335) return BOTAO_DICA;
    if (x >= 480 && x <= 565) return BOTAO_SELECIONAR_FASES;
    if (x >= 575 && x <= 655) return BOTAO_MENU;
    if (x >= 665 && x <= 790) return BOTAO_REINICIAR;
    if (jogo->estado == CHEGOU && jogo_tem_proxima_fase(jogo)
        && x >= 345 && x <= 465) return BOTAO_PROXIMA_FASE;
    return 0;
}

static void desenhar_botoes(const Jogo *jogo, ALLEGRO_FONT *fonte)
{
    desenhar_botao(fonte, 250, 335, "DICA", jogo->mostrar_dica
        ? al_map_rgb(190, 145, 20) : al_map_rgb(120, 95, 30));
    if (jogo->estado == CHEGOU && jogo_tem_proxima_fase(jogo))
        desenhar_botao(fonte, 345, 465, "PROXIMA", al_map_rgb(28, 145, 92));
    desenhar_botao(fonte, 480, 565, "FASES", al_map_rgb(48, 105, 155));
    desenhar_botao(fonte, 575, 655, "MENU", al_map_rgb(75, 80, 105));
    desenhar_botao(fonte, 665, 790, "REINICIAR", al_map_rgb(145, 82, 55));
}

static void desenhar_eixos(ALLEGRO_FONT *fonte, int indice_fase)
{
    double y_min = indice_fase == 1 ? FASE2_Y_MIN : Y_MIN;
    double y_max = indice_fase == 1 ? FASE2_Y_MAX : Y_MAX;
    ALLEGRO_COLOR cor_eixo = al_map_rgb(235, 240, 245);
    ALLEGRO_COLOR cor_texto = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR cor_grade = al_map_rgba(255, 255, 255, 95);
    const int quantidade_marcas = 5;

    /* Eixo Y e valores na lateral esquerda. */
    al_draw_line(PISTA_X1, PISTA_Y1, PISTA_X1, PISTA_Y2, cor_eixo, 2);
    for (int i = 0; i <= quantidade_marcas; i++) {
        double valor = y_min + (y_max - y_min) * i / quantidade_marcas;
        float py = PISTA_Y2 - (float)i * (PISTA_Y2 - PISTA_Y1) / quantidade_marcas;
        al_draw_line(PISTA_X1 - 4, py, PISTA_X1 + 4, py, cor_eixo, 1);
        al_draw_textf(fonte, cor_texto, PISTA_X1 - 8,
            py - al_get_font_line_height(fonte) / 2, ALLEGRO_ALIGN_RIGHT, "%.1f", valor);
    }
    al_draw_text(fonte, cor_texto, 8, PISTA_Y1 - 2, ALLEGRO_ALIGN_LEFT, "Y");

    /* Eixo X e valores abaixo da pista. */
    al_draw_line(PISTA_X1, PISTA_Y2, PISTA_X2, PISTA_Y2, cor_eixo, 2);
    for (int i = 0; i <= 10; i += 2) {
        float px = PISTA_X1 + (float)i * (PISTA_X2 - PISTA_X1) / 10.0f;
        al_draw_line(px, PISTA_Y2 - 4, px, PISTA_Y2 + 4, cor_eixo, 1);
        al_draw_line(px, PISTA_Y1, px, PISTA_Y2, cor_grade, 1);
        al_draw_textf(fonte, cor_texto, px, PISTA_Y2 + 5, ALLEGRO_ALIGN_CENTRE, "%d", i);
    }
    al_draw_text(fonte, cor_texto, PISTA_X2 - 5, PISTA_Y2 + 5,
        ALLEGRO_ALIGN_RIGHT, "X");
}

static void desenhar_numero_da_fase(const Jogo *jogo, ALLEGRO_FONT *fonte)
{
    int altura = al_get_font_line_height(fonte);

    al_draw_filled_rectangle(PISTA_X1, PISTA_Y1, PISTA_X1 + 90, PISTA_Y1 + altura + 10,
        al_map_rgba(0, 0, 0, 180));
    al_draw_textf(fonte, al_map_rgb(255, 255, 255), PISTA_X1 + 45, PISTA_Y1 + 5,
        ALLEGRO_ALIGN_CENTRE, "FASE %d", jogo->fase + 1);
}

/* Caixa com a dica da fase, em cima da pista (aparece pelo botao DICA) */
static void desenhar_dica(const Jogo *jogo, ALLEGRO_FONT *fonte)
{
    ALLEGRO_COLOR amarelo = al_map_rgb(255, 215, 60);
    int altura = al_get_font_line_height(fonte);

    al_draw_filled_rounded_rectangle(200, PISTA_Y1 + 10, 620, PISTA_Y1 + altura + 26,
        8, 8, al_map_rgba(0, 0, 0, 210));
    al_draw_rounded_rectangle(200, PISTA_Y1 + 10, 620, PISTA_Y1 + altura + 26,
        8, 8, amarelo, 2);
    al_draw_text(fonte, amarelo, LARGURA / 2, PISTA_Y1 + 18, ALLEGRO_ALIGN_CENTRE,
        fases_obter(jogo->fase)->dica);
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
        al_draw_text(fonte, cinza, x, y, ALLEGRO_ALIGN_LEFT, fases_obter(jogo->fase)->instrucao);
    } else if (jogo->estado == ANDANDO && jogo->saiu_do_asfalto) {
        al_draw_textf(fonte, vermelho, x, y, ALLEGRO_ALIGN_LEFT,
            "x = %.2f    f(x) = %.2f    FORA DA PISTA desde x = %.2f",
            jogo->x, jogo->y, jogo->x_saida);
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

static void desenhar_cartao_fase(int indice, int fase_atual, ALLEGRO_FONT *fonte)
{
    const int x = 30 + indice * 190;
    const int y = 165;
    const int w = 170;
    const int h = 300;
    const char *nomes[] = { "A RETA", "A SUBIDA", "A PARABOLA", "O SLALOM" };
    const char *descricoes[] = {
        "Domine a reta",
        "Encare a subida",
        "Curva em U",
        "Desvie dos cones"
    };
    ALLEGRO_BITMAP *imagem = fases_imagem(indice);
    ALLEGRO_COLOR painel = indice == fase_atual
        ? al_map_rgb(34, 75, 83) : al_map_rgb(31, 42, 61);
    ALLEGRO_COLOR destaque = indice == fase_atual
        ? al_map_rgb(85, 220, 170) : al_map_rgb(76, 145, 190);

    al_draw_filled_rounded_rectangle(x + 3, y + 6, x + w + 3, y + h + 6,
        16, 16, al_map_rgba(0, 0, 0, 125));
    al_draw_filled_rounded_rectangle(x, y, x + w, y + h, 16, 16, painel);
    al_draw_rounded_rectangle(x, y, x + w, y + h, 16, 16, destaque, 2);

    if (imagem) {
        al_draw_filled_rounded_rectangle(x + 10, y + 10, x + w - 10,
            y + 132, 9, 9, al_map_rgb(15, 22, 32));
        al_draw_scaled_bitmap(imagem, 0, 0,
            (float)al_get_bitmap_width(imagem), (float)al_get_bitmap_height(imagem),
            x + 10, y + 10, w - 20, 122, 0);
    }

    al_draw_textf(fonte, al_map_rgb(110, 205, 245), x + 15, y + 145,
        ALLEGRO_ALIGN_LEFT, "FASE %d", indice + 1);
    al_draw_text(fonte, al_map_rgb(255, 255, 255), x + 15, y + 171,
        ALLEGRO_ALIGN_LEFT, nomes[indice]);
    al_draw_text(fonte, al_map_rgb(185, 198, 215), x + 15, y + 202,
        ALLEGRO_ALIGN_LEFT, descricoes[indice]);

    ALLEGRO_COLOR botao = indice == fase_atual
        ? al_map_rgb(35, 165, 120) : al_map_rgb(47, 115, 170);
    al_draw_filled_rounded_rectangle(x + 15, y + 244, x + w - 15,
        y + 280, 9, 9, botao);
    al_draw_rounded_rectangle(x + 15, y + 244, x + w - 15,
        y + 280, 9, 9, destaque, 1);
    al_draw_text(fonte, al_map_rgb(255, 255, 255), x + w / 2, y + 253,
        ALLEGRO_ALIGN_CENTRE, "JOGAR FASE");
}

int desenho_identificar_fase(int x, int y)
{
    if (y < 165 || y > 465) return -1;
    for (int i = 0; i < fases_quantidade(); i++) {
        int x1 = 30 + i * 190;
        if (x >= x1 && x <= x1 + 170) return i;
    }
    return -1;
}

bool desenho_clicou_voltar_fases(int x, int y)
{
    return x >= 300 && x <= 500 && y >= 500 && y <= 550;
}

void desenho_tela_fases(int fase_atual, ALLEGRO_FONT *fonte)
{
    al_clear_to_color(al_map_rgb(14, 20, 34));
    al_draw_filled_rectangle(0, 0, LARGURA, 145, al_map_rgb(22, 34, 54));
    al_draw_text(fonte, al_map_rgb(100, 220, 185), LARGURA / 2, 38,
        ALLEGRO_ALIGN_CENTRE, "ESCOLHA SUA PROXIMA CORRIDA");
    al_draw_text(fonte, al_map_rgb(205, 215, 230), LARGURA / 2, 72,
        ALLEGRO_ALIGN_CENTRE, "Selecione uma pista para comecar a jogar");

    for (int i = 0; i < fases_quantidade(); i++)
        desenhar_cartao_fase(i, fase_atual, fonte);

    al_draw_filled_rounded_rectangle(300, 500, 500, 550, 12, 12,
        al_map_rgb(65, 73, 96));
    al_draw_rounded_rectangle(300, 500, 500, 550, 12, 12,
        al_map_rgb(135, 155, 185), 1);
    al_draw_text(fonte, al_map_rgb(255, 255, 255), LARGURA / 2, 516,
        ALLEGRO_ALIGN_CENTRE, "VOLTAR AO MENU");
    al_flip_display();
}

void desenhar_tela(const Jogo *jogo, const ALLEGRO_USTR *texto,
                   ALLEGRO_BITMAP *carro, ALLEGRO_FONT *fonte)
{
    const Fase *fase = fases_obter(jogo->fase);
    ALLEGRO_BITMAP *pista = fases_imagem(jogo->fase);

    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (pista) {
        al_draw_scaled_bitmap(pista, 0, 0,
            (float)al_get_bitmap_width(pista), (float)al_get_bitmap_height(pista),
            PISTA_X1, PISTA_Y1, PISTA_X2 - PISTA_X1, PISTA_Y2 - PISTA_Y1, 0);
    }
    desenhar_eixos(fonte, jogo->fase);
    if (jogo->funcao) desenhar_funcao(jogo->funcao, al_map_rgb(255, 200, 0), jogo->fase);
    obstaculos_desenhar(fase->obstaculos, fase->num_obstaculos, jogo->fase);
    desenhar_carro(jogo, carro);
    desenhar_numero_da_fase(jogo, fonte);
    desenhar_botoes(jogo, fonte);
    if (jogo->mostrar_dica) desenhar_dica(jogo, fonte);
    if (jogo->estado == CHEGOU) desenhar_vitoria(jogo, fonte);
    desenhar_mensagem(jogo, fonte);
    campo_desenhar(texto, fonte);
    al_flip_display();
}
