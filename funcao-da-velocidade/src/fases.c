#include <math.h>
#include <stdio.h>
#include <allegro5/allegro_image.h>
#include "fases.h"
#include "config.h"

/* Fase 1: reta */
static double centro_reta(double x)
{
    (void)x;
    return 0.0;
}

static const Obstaculo obstaculos_fase1[] = {
    { OBSTACULO_PEDRA,  2.0, -0.2, 42.0f, 36.0f },
    { OBSTACULO_CONE,   5.1, -1.1, 40.0f, 38.0f },
    { OBSTACULO_BURACO, 8.0, -1.8, 48.0f, 34.0f }
};

/* Fase 2: subida, y = 0.8x - 4 */
static double centro_subida(double x)
{
    return 0.8 * x - 4.0;
}

static const Obstaculo obstaculos_fase2[] = {
    { OBSTACULO_PEDRA,  2.6, -0.37, 32.0f, 28.0f },
    { OBSTACULO_CONE,   5.0, -1.55, 30.0f, 28.0f },
    { OBSTACULO_BURACO, 7.0,  3.15, 36.0f, 26.0f }
};

/* Fase 3: parábola, y = (x - 5)^2 / 5 - 3 */
static double centro_parabola(double x)
{
    return (x - 5.0) * (x - 5.0) / 5.0 - 3.0;
}

static const Obstaculo obstaculos_fase3[] = {
    { OBSTACULO_PEDRA,  2.0, -2.90,  32.0f, 28.0f },
    { OBSTACULO_CONE,   5.0, -1.25,  30.0f, 28.0f },
    { OBSTACULO_BURACO, 7.3, -0.342, 36.0f, 26.0f }
};

#define QTD(v) ((int)(sizeof(v) / sizeof((v)[0])))

static const Fase fases[] = {
    { "images/pista1.png", centro_reta,     obstaculos_fase1, QTD(obstaculos_fase1),
      "Digite uma funcao de x e aperte ENTER. Exemplos: sen(x)   x/2   2sen(x)+1" },
    { "images/pista2.png", centro_subida,   obstaculos_fase2, QTD(obstaculos_fase2),
      "A estrada agora e uma subida em linha reta. Qual funcao sobe assim?" },
    { "images/pista3.png", centro_parabola, obstaculos_fase3, QTD(obstaculos_fase3),
      "A estrada desce e sobe em forma de U, como uma parabola." }
};

#define NUM_FASES QTD(fases)

static ALLEGRO_BITMAP *imagens[NUM_FASES];

bool fases_inicializar(void)
{
    for (int i = 0; i < NUM_FASES; i++) {
        imagens[i] = al_load_bitmap(fases[i].imagem);
        if (!imagens[i]) {
            fprintf(stderr, "Falha ao carregar %s (confira a pasta images).\n", fases[i].imagem);
            fases_finalizar();
            return false;
        }
    }
    return true;
}

void fases_finalizar(void)
{
    for (int i = 0; i < NUM_FASES; i++) {
        if (imagens[i]) al_destroy_bitmap(imagens[i]);
        imagens[i] = NULL;
    }
}

int fases_quantidade(void)
{
    return NUM_FASES;
}

const Fase *fases_obter(int indice)
{
    return &fases[indice];
}

ALLEGRO_BITMAP *fases_imagem(int indice)
{
    return imagens[indice];
}

bool fase_no_asfalto(const Fase *fase, double x, double y)
{
    return fabs(y - fase->centro(x)) <= ASFALTO_MEIA_ALTURA;
}
