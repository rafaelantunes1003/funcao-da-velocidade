#include <math.h>
#include <stdbool.h>
#include "jogo.h"
#include "config.h"
#include "coordenadas.h"

static bool dentro_da_pista(double y)
{
    return isfinite(y) && y >= Y_MIN && y <= Y_MAX;
}

static float angulo_na_curva(const Expressao *funcao, double x, float angulo_atual)
{
    double passo = 0.01;
    double y1 = expressao_avaliar(funcao, x);
    double y2 = expressao_avaliar(funcao, x + passo);
    if (!isfinite(y1) || !isfinite(y2)) return angulo_atual;

    float dx = tela_x(x + passo) - tela_x(x);
    float dy = tela_y(y2) - tela_y(y1);
    return atan2f(dy, dx) + CARRO_DIREITA;
}

static void sair_da_pista(Jogo *jogo, double x, double y)
{
    jogo->x_saida = x;
    jogo->y_saida = y;
    jogo->estado = SAIU;
}

void jogo_iniciar(Jogo *jogo)
{
    jogo->funcao = NULL;
    jogo->erro[0] = '\0';
    jogo->estado = PARADO;
    jogo->passos = 0;
    jogo->x = X_MIN;
    jogo->y = 0;
    jogo->x_saida = 0;
    jogo->y_saida = 0;
    jogo->angulo = CARRO_DIREITA;
    jogo->obstaculo_colisao = NULL;
}

void jogo_largar(Jogo *jogo, const char *texto)
{
    Expressao *nova = expressao_compilar(texto, jogo->erro, sizeof(jogo->erro));
    expressao_liberar(jogo->funcao);
    jogo->funcao = nova;
    jogo->passos = 0;
    jogo->x = X_MIN;
    jogo->y = 0;
    jogo->angulo = CARRO_DIREITA;
    jogo->obstaculo_colisao = NULL;

    if (!jogo->funcao) {
        jogo->estado = PARADO;
        return;
    }

    double y_inicio = expressao_avaliar(jogo->funcao, jogo->x);
    if (dentro_da_pista(y_inicio)) {
        jogo->y = y_inicio;
        jogo->angulo = angulo_na_curva(jogo->funcao, jogo->x, jogo->angulo);
        jogo->estado = ANDANDO;
    } else {
        sair_da_pista(jogo, jogo->x, y_inicio);
    }
}

void jogo_atualizar(Jogo *jogo)
{
    if (jogo->estado != ANDANDO) return;

    double novo_x = X_MIN + (jogo->passos + 1) * VELOCIDADE / FPS;
    if (novo_x > X_MAX) novo_x = X_MAX;
    double novo_y = expressao_avaliar(jogo->funcao, novo_x);

    if (!dentro_da_pista(novo_y)) {
        sair_da_pista(jogo, novo_x, novo_y);
        return;
    }

    const Obstaculo *colisao = obstaculos_verificar_colisao(
        jogo->x, jogo->y, novo_x, novo_y);
    if (colisao) {
        jogo->x = novo_x;
        jogo->y = novo_y;
        jogo->obstaculo_colisao = colisao;
        jogo->estado = COLIDIU;
        return;
    }

    jogo->passos++;
    jogo->x = novo_x;
    jogo->y = novo_y;
    jogo->angulo = angulo_na_curva(jogo->funcao, jogo->x, jogo->angulo);
    if (jogo->x >= X_MAX) jogo->estado = CHEGOU;
}

void jogo_finalizar(Jogo *jogo)
{
    expressao_liberar(jogo->funcao);
    jogo->funcao = NULL;
}
