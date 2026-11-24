#include <math.h>
#include <stdbool.h>
#include "jogo.h"
#include "config.h"
#include "coordenadas.h"
#include "fases.h"

/* O carro precisa estar dentro da tela e em cima do asfalto da fase */
static bool dentro_da_pista(const Jogo *jogo, double x, double y)
{
    double minimo = jogo->fase == 1 ? FASE2_Y_MIN : Y_MIN;
    double maximo = jogo->fase == 1 ? FASE2_Y_MAX : Y_MAX;
    return isfinite(y) && y >= minimo && y <= maximo
        && fase_no_asfalto(fases_obter(jogo->fase), x, y);
}

static float angulo_na_curva(const Expressao *funcao, double x, float angulo_atual, int indice_fase)
{
    double passo = 0.01;
    double y1 = expressao_avaliar(funcao, x);
    double y2 = expressao_avaliar(funcao, x + passo);
    if (!isfinite(y1) || !isfinite(y2)) return angulo_atual;

    float dx = tela_x(x + passo) - tela_x(x);
    float dy = tela_y_fase(y2, indice_fase) - tela_y_fase(y1, indice_fase);
    return atan2f(dy, dx) + CARRO_DIREITA;
}

/* Põe o carro parado no começo da estrada, virado para o mesmo lado dela */
static void colocar_na_largada(Jogo *jogo)
{
    const Fase *fase = fases_obter(jogo->fase);
    double passo = 0.01;
    double y1 = fase->centro(X_MIN);
    double y2 = fase->centro(X_MIN + passo);
    float dx = tela_x(X_MIN + passo) - tela_x(X_MIN);
    float dy = tela_y_fase(y2, jogo->fase) - tela_y_fase(y1, jogo->fase);

    jogo->x = X_MIN;
    jogo->y = y1;
    jogo->angulo = atan2f(dy, dx) + CARRO_DIREITA;
}

static void sair_da_pista(Jogo *jogo, double x, double y)
{
    jogo->x_saida = x;
    jogo->y_saida = y;
    jogo->estado = SAIU;
}

void jogo_iniciar(Jogo *jogo)
{
    jogo->fase = 0;
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

void jogo_reiniciar_fase(Jogo *jogo)
{
    expressao_liberar(jogo->funcao);
    jogo->funcao = NULL;
    jogo->erro[0] = '\0';
    jogo->estado = PARADO;
    jogo->passos = 0;
    jogo->x_saida = 0;
    jogo->y_saida = 0;
    jogo->obstaculo_colisao = NULL;
    colocar_na_largada(jogo);
}

void jogo_largar(Jogo *jogo, const char *texto)
{
    Expressao *nova = expressao_compilar(texto, jogo->erro, sizeof(jogo->erro));
    expressao_liberar(jogo->funcao);
    jogo->funcao = nova;
    jogo->passos = 0;
    colocar_na_largada(jogo);
    jogo->obstaculo_colisao = NULL;

    if (!jogo->funcao) {
        jogo->estado = PARADO;
        return;
    }

    double y_inicio = expressao_avaliar(jogo->funcao, jogo->x);
    if (dentro_da_pista(jogo, jogo->x, y_inicio)) {
        jogo->y = y_inicio;
        jogo->angulo = angulo_na_curva(jogo->funcao, jogo->x, jogo->angulo, jogo->fase);
        jogo->estado = ANDANDO;
    } else {
        sair_da_pista(jogo, jogo->x, y_inicio);
    }
}

void jogo_atualizar(Jogo *jogo)
{
    if (jogo->estado != ANDANDO) return;

    double novo_x = X_MIN + (jogo->passos + 1) * VELOCIDADE / FPS;
    if (novo_x > X_CHEGADA) novo_x = X_CHEGADA;
    double novo_y = expressao_avaliar(jogo->funcao, novo_x);

    if (!dentro_da_pista(jogo, novo_x, novo_y)) {
        sair_da_pista(jogo, novo_x, novo_y);
        return;
    }

    const Fase *fase = fases_obter(jogo->fase);
    const Obstaculo *colisao = obstaculos_verificar_colisao(
        fase->obstaculos, fase->num_obstaculos, jogo->fase,
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
    jogo->angulo = angulo_na_curva(jogo->funcao, jogo->x, jogo->angulo, jogo->fase);
    if (jogo->x >= X_CHEGADA) {
        jogo->estado = CHEGOU;
    }
}

bool jogo_tem_proxima_fase(const Jogo *jogo)
{
    return jogo->fase + 1 < fases_quantidade();
}

void jogo_proxima_fase(Jogo *jogo)
{
    int proxima = jogo->fase + 1;
    jogo_finalizar(jogo);
    jogo_iniciar(jogo);
    jogo->fase = proxima;
    colocar_na_largada(jogo);
}

void jogo_finalizar(Jogo *jogo)
{
    expressao_liberar(jogo->funcao);
    jogo->funcao = NULL;
}
