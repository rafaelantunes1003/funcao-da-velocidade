#include <math.h>
#include <stddef.h>
#include <allegro5/allegro_primitives.h>
#include "obstaculos.h"
#include "coordenadas.h"
#include "config.h"

/* Posições de exemplo em coordenadas do gráfico. Ajuste x/y para reposicionar. */
static const Obstaculo obstaculos[] = {
    { OBSTACULO_PEDRA, 2.2,  1.5, 17.0f, 13.0f },
    { OBSTACULO_CONE,  5.0, -1.8, 11.0f, 19.0f },
    { OBSTACULO_BURACO, 7.8,  2.4, 25.0f, 11.0f }
};

const Obstaculo *obstaculos_obter(int *quantidade)
{
    if (quantidade) *quantidade = (int)(sizeof(obstaculos) / sizeof(obstaculos[0]));
    return obstaculos;
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

const Obstaculo *obstaculos_verificar_colisao(double x_anterior, double y_anterior,
                                               double x_atual, double y_atual)
{
    const float meia_largura_carro = 14.0f, meia_altura_carro = 7.0f;
    float x0 = tela_x(x_anterior), y0 = tela_y(y_anterior);
    float x1 = tela_x(x_atual), y1 = tela_y(y_atual);

    for (size_t i = 0; i < sizeof(obstaculos) / sizeof(obstaculos[0]); i++) {
        const Obstaculo *o = &obstaculos[i];
        float cx = tela_x(o->x), cy = tela_y(o->y);
        float mx = o->meia_largura_px + meia_largura_carro;
        float my = o->meia_altura_px + meia_altura_carro;
        if (segmento_intersecta_retangulo(x0, y0, x1, y1,
                                          cx - mx, cy - my, cx + mx, cy + my))
            return o;
    }
    return NULL;
}

void obstaculos_desenhar(void)
{
    for (size_t i = 0; i < sizeof(obstaculos) / sizeof(obstaculos[0]); i++) {
        const Obstaculo *o = &obstaculos[i];
        float x = tela_x(o->x), y = tela_y(o->y);
        switch (o->tipo) {
        case OBSTACULO_PEDRA:
            al_draw_filled_ellipse(x, y, o->meia_largura_px, o->meia_altura_px,
                                   al_map_rgb(125, 125, 130));
            al_draw_ellipse(x, y, o->meia_largura_px, o->meia_altura_px,
                            al_map_rgb(190, 190, 195), 2.0f);
            break;
        case OBSTACULO_CONE:
            al_draw_filled_triangle(x, y - o->meia_altura_px,
                x - o->meia_largura_px, y + o->meia_altura_px,
                x + o->meia_largura_px, y + o->meia_altura_px,
                al_map_rgb(245, 115, 25));
            al_draw_triangle(x, y - o->meia_altura_px,
                x - o->meia_largura_px, y + o->meia_altura_px,
                x + o->meia_largura_px, y + o->meia_altura_px,
                al_map_rgb(255, 210, 120), 2.0f);
            break;
        case OBSTACULO_BURACO:
            al_draw_filled_ellipse(x, y, o->meia_largura_px, o->meia_altura_px,
                                   al_map_rgb(105, 80, 55));
            al_draw_filled_ellipse(x, y, o->meia_largura_px * 0.82f,
                o->meia_altura_px * 0.72f, al_map_rgb(18, 18, 22));
            break;
        }
    }
}
