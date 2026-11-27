#include <math.h>
#include <stdbool.h>
#include "jogo.h"
#include "config.h"
#include "coordenadas.h"
#include "fases.h"

/* O ponto existe e aparece na tela? (fora disso o carro precisa parar) */
static bool dentro_da_tela(const Jogo *jogo, double y)
{
    double minimo = jogo->fase == 1 ? FASE2_Y_MIN : Y_MIN;
    double maximo = jogo->fase == 1 ? FASE2_Y_MAX : Y_MAX;
    return isfinite(y) && y >= minimo && y <= maximo;
}

/* Guarda o primeiro ponto em que o carro saiu do asfalto.
 * O carro continua andando em cima do gráfico; só no fim conta como derrota. */
static void marcar_saida_do_asfalto(Jogo *jogo, double x, double y)
{
    if (jogo->saiu_do_asfalto) return;
    jogo->saiu_do_asfalto = true;
    jogo->x_saida = x;
    jogo->y_saida = y;
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

/* Para o carro: o gráfico saiu da tela ou a função não existe nesse x */
static void sair_da_pista(Jogo *jogo, double x, double y)
{
    marcar_saida_do_asfalto(jogo, x, y);
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
    jogo->mostrar_dica = false;
    jogo->saiu_do_asfalto = false;
}

void jogo_reiniciar_fase(Jogo *jogo)
{
    jogo->mostrar_dica = false;
    jogo->saiu_do_asfalto = false;
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
    jogo->saiu_do_asfalto = false;

    if (!jogo->funcao) {
        jogo->estado = PARADO;
        return;
    }

    double y_inicio = expressao_avaliar(jogo->funcao, jogo->x);
    if (!dentro_da_tela(jogo, y_inicio)) {
        sair_da_pista(jogo, jogo->x, y_inicio);
        return;
    }

    jogo->y = y_inicio;
    jogo->angulo = angulo_na_curva(jogo->funcao, jogo->x, jogo->angulo, jogo->fase);
    jogo->estado = ANDANDO;
    if (!fase_no_asfalto(fases_obter(jogo->fase), jogo->x, jogo->y)) {
        marcar_saida_do_asfalto(jogo, jogo->x, jogo->y);
    }
}

void jogo_atualizar(Jogo *jogo)
{
    if (jogo->estado != ANDANDO) return;

    double novo_x = X_MIN + (jogo->passos + 1) * VELOCIDADE / FPS;
    if (novo_x > X_CHEGADA) novo_x = X_CHEGADA;
    double novo_y = expressao_avaliar(jogo->funcao, novo_x);

    if (!dentro_da_tela(jogo, novo_y)) {
        sair_da_pista(jogo, novo_x, novo_y);
        return;
    }

    const Fase *fase = fases_obter(jogo->fase);
    if (!fase_no_asfalto(fase, novo_x, novo_y)) {
        marcar_saida_do_asfalto(jogo, novo_x, novo_y);
    }
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
        /* Completou o trajeto: só vence se não saiu do asfalto no caminho */
        jogo->estado = jogo->saiu_do_asfalto ? SAIU : CHEGOU;
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
