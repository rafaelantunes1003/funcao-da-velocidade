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
    double minimo = indice_fase == 1 ? FASE2_Y_MIN : Y_MIN;
    double maximo = indice_fase == 1 ? FASE2_Y_MAX : Y_MAX;
    return (float)(PISTA_Y2 - (y - minimo) * (PISTA_Y2 - PISTA_Y1) / (maximo - minimo));
}
