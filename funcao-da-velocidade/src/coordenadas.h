#ifndef COORDENADAS_H
#define COORDENADAS_H

/* Converte um ponto do gráfico (x, y) para pixels da tela */
float tela_x(double x);
float tela_y(double y);
float tela_y_fase(double y, int indice_fase);

#endif
