#ifndef EXPRESSAO_H
#define EXPRESSAO_H

#include <stddef.h>

/*
 * Expressão matemática já "compilada", com a variável x.
 *
 * Aceita: numeros (2, 0.5, 0,5), x, pi, e, + - * / ^, parenteses,
 * multiplicacao implicita (2x, 3sen(x), 2(x+1)) e as funcoes
 * sen/sin, cos, tan/tg, asin, acos, atan, sqrt/raiz, abs, ln, log, exp.
 * Os angulos sao em radianos. Maiusculas e minusculas tanto faz.
 */
typedef struct Expressao Expressao;

/*
 * Compila o texto digitado pelo jogador.
 * Retorna NULL se houver erro de sintaxe e, nesse caso, escreve uma
 * mensagem curta em 'erro' (se 'erro' nao for NULL).
 * Quem recebe o ponteiro deve chamar expressao_liberar().
 */
Expressao* expressao_compilar(const char* texto, char* erro, size_t erro_tam);

/*
 * Calcula o valor da expressao para um dado x.
 * Pode devolver NaN ou infinito (ex.: sqrt(-1), 1/0): use isfinite()
 * para saber se o resultado e valido.
 */
double expressao_avaliar(const Expressao* e, double x);

/* Libera a memoria da expressao (aceita NULL). */
void expressao_liberar(Expressao* e);

/* ------------------------------------------------------------------ */
/* Ajuda: tudo o que o jogador pode digitar, para a tela de ajuda.     */
/* A lista sai da mesma tabela que o interpretador usa, entao ela      */
/* nunca fica desatualizada.                                           */
/* ------------------------------------------------------------------ */
typedef struct {
    const char* titulo;     /* o que se digita, ex.: "sen" ou "+  -"      */
    const char* exemplo;    /* exemplo que compila, ex.: "sen(x)"         */
    const char* descricao;  /* explicacao curta, ex.: "seno, angulo..."   */
    int eh_funcao;          /* 1 = funcao (sen, cos...), 0 = simbolo      */
} InfoAjuda;

/* Quantos itens a ajuda tem (simbolos primeiro, depois as funcoes). */
int expressao_num_ajuda(void);

/* Item 'indice' (0 ate num-1). Devolve NULL se fora do intervalo.
 * O ponteiro e fixo: nao precisa liberar. */
const InfoAjuda* expressao_ajuda(int indice);

#endif