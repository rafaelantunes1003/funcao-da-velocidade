#include <math.h>
#include <stddef.h>
#include <allegro5/allegro_image.h>
#include "obstaculos.h"
#include "coordenadas.h"
#include "config.h"

/* As posições dos obstáculos de cada fase ficam em fases.c */

static ALLEGRO_BITMAP *imagem_pedra = NULL;
static ALLEGRO_BITMAP *imagem_cone = NULL;
static ALLEGRO_BITMAP *imagem_buraco = NULL;

bool obstaculos_inicializar(void)
{
    imagem_pedra = al_load_bitmap("images/pedra.png");
    imagem_cone = al_load_bitmap("images/cone.png");
    imagem_buraco = al_load_bitmap("images/buraco.png");

    if (!imagem_pedra || !imagem_cone || !imagem_buraco) {
        obstaculos_finalizar();
        return false;
    }
    return true;
}

void obstaculos_finalizar(void)
{
    if (imagem_pedra) al_destroy_bitmap(imagem_pedra);
    if (imagem_cone) al_destroy_bitmap(imagem_cone);
    if (imagem_buraco) al_destroy_bitmap(imagem_buraco);

    imagem_pedra = NULL;
    imagem_cone = NULL;
    imagem_buraco = NULL;
}

/* Interseção segmento-retângulo: evita que o carro atravesse objetos entre frames. */
static bool segmento_intersecta_retangulo(float x0, float y0, float x1, float y1,
                                           float esquerda, float topo,
                                           float direita, float base)
{
    float t_min = 0.0f, t_max = 1.0f;
    float dx = x1 - x0, dy = y1 - y0;

    if (fabsf(dx) < 0.00001f) {
        if (x0 < esquerda || x0 > direita) return false;
    } else {
        float a = (esquerda - x0) / dx, b = (direita - x0) / dx;
        if (a > b) { float t = a; a = b; b = t; }
        if (a > t_min) t_min = a;
        if (b < t_max) t_max = b;
        if (t_min > t_max) return false;
    }

    if (fabsf(dy) < 0.00001f) {
        if (y0 < topo || y0 > base) return false;
    } else {
        float a = (topo - y0) / dy, b = (base - y0) / dy;
        if (a > b) { float t = a; a = b; b = t; }
        if (a > t_min) t_min = a;
        if (b < t_max) t_max = b;
        if (t_min > t_max) return false;
    }
    return true;
}

const Obstaculo *obstaculos_verificar_colisao(const Obstaculo *obstaculos, int quantidade,
                                               int indice_fase,
                                               double x_anterior, double y_anterior,
                                               double x_atual, double y_atual)
{
    const float meia_largura_carro = 14.0f, meia_altura_carro = 7.0f;
    float x0 = tela_x(x_anterior), y0 = tela_y_fase(y_anterior, indice_fase);
    float x1 = tela_x(x_atual), y1 = tela_y_fase(y_atual, indice_fase);

    for (int i = 0; i < quantidade; i++) {
        const Obstaculo *o = &obstaculos[i];
        float cx = tela_x(o->x), cy = tela_y_fase(o->y, indice_fase);
        float mx = o->meia_largura_px + meia_largura_carro;
        float my = o->meia_altura_px + meia_altura_carro;
        if (segmento_intersecta_retangulo(x0, y0, x1, y1,
                                          cx - mx, cy - my, cx + mx, cy + my))
            return o;
    }
    return NULL;
}

/* Mantém a proporção original e encaixa a imagem na área definida para o obstáculo. */
static void desenhar_imagem_obstaculo(ALLEGRO_BITMAP *imagem,
                                      const Obstaculo *obstaculo, int indice_fase)
{
    if (!imagem) return;

    float largura_original = (float)al_get_bitmap_width(imagem);
    float altura_original = (float)al_get_bitmap_height(imagem);
    float largura_limite = obstaculo->meia_largura_px * 2.0f;
    float altura_limite = obstaculo->meia_altura_px * 2.0f;
    float escala_x = largura_limite / largura_original;
    float escala_y = altura_limite / altura_original;
    float escala = escala_x < escala_y ? escala_x : escala_y;
    float largura = largura_original * escala;
    float altura = altura_original * escala;
    float x = tela_x(obstaculo->x);
    float y = tela_y_fase(obstaculo->y, indice_fase);

    al_draw_scaled_bitmap(imagem, 0, 0, largura_original, altura_original,
        x - largura / 2.0f, y - altura / 2.0f, largura, altura, 0);
}

void obstaculos_desenhar(const Obstaculo *obstaculos, int quantidade, int indice_fase)
{
    for (int i = 0; i < quantidade; i++) {
        const Obstaculo *o = &obstaculos[i];
        switch (o->tipo) {
        case OBSTACULO_PEDRA:
            desenhar_imagem_obstaculo(imagem_pedra, o, indice_fase);
            break;
        case OBSTACULO_CONE:
            desenhar_imagem_obstaculo(imagem_cone, o, indice_fase);
            break;
        case OBSTACULO_BURACO:
            desenhar_imagem_obstaculo(imagem_buraco, o, indice_fase);
            break;
        }
    }
}
