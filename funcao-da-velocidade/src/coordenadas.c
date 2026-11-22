#include "coordenadas.h"
#include "config.h"

float tela_x(double x)
{
    return (float)(PISTA_X1 + (x - X_MIN) * (PISTA_X2 - PISTA_X1) / (X_MAX - X_MIN));
}

float tela_y(double y)
{
    return tela_y_fase(y, 0);
}

float tela_y_fase(double y, int indice_fase)
{
    double minimo = Y_MIN;
    double maximo = Y_MAX;

    if (indice_fase == 1) {
        minimo = FASE2_Y_MIN;
        maximo = FASE2_Y_MAX;
    } else if (indice_fase == 2) {
        minimo = FASE3_Y_MIN;
        maximo = FASE3_Y_MAX;
    }
    return (float)(PISTA_Y2 - (y - minimo) * (PISTA_Y2 - PISTA_Y1) / (maximo - minimo));
}
