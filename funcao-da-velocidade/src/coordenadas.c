#include "coordenadas.h"
#include "config.h"

float tela_x(double x)
{
    return (float)(PISTA_X1 + (x - X_MIN) * (PISTA_X2 - PISTA_X1) / (X_MAX - X_MIN));
}

float tela_y(double y)
{
    return (float)(PISTA_Y2 - (y - Y_MIN) * (PISTA_Y2 - PISTA_Y1) / (Y_MAX - Y_MIN));
}
