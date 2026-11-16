/*
 * expressao.c - interpretador de expressões matemáticas com a variável x.
 *
 * Gramática (da menor para a maior precedência):
 *   expr     := termo (('+' | '-') termo)*
 *   termo    := unario (('*' | '/' | <multiplicação implícita>) unario)*
 *   unario   := ('-' | '+') unario | potencia
 *   potencia := primario ('^' unario)?          (associa à direita)
 *   primario := numero | ident | ident '(' expr ')' | '(' expr ')'
 * */
#include "expressao.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TEXTO         256   /* tamanho máximo do texto digitado */
#define MAX_PROFUNDIDADE  64    /* limite de parênteses/potências aninhados */
#define MAX_IDENT         16    /* tamanho máximo de um nome (sen, sqrt...) */

#define VALOR_PI 3.14159265358979323846
#define VALOR_E  2.71828182845904523536

 /* ------------------------------------------------------------------ */
 /* Árvore da expressão                                                  */
 /* ------------------------------------------------------------------ */

typedef enum { NO_NUM, NO_VAR, NO_NEG, NO_BIN, NO_FUNC } TipoNo;

typedef enum {
    F_SIN, F_COS, F_TAN, F_ASIN, F_ACOS, F_ATAN,
    F_SQRT, F_ABS, F_LN, F_LOG, F_EXP
} Funcao;

typedef struct No {
    TipoNo tipo;
    double valor;      /* NO_NUM */
    char op;           /* NO_BIN: + - * / ^ */
    Funcao funcao;     /* NO_FUNC */
    struct No* esq;    /* operando esquerdo (ou único operando) */
    struct No* dir;    /* operando direito */
} No;

struct Expressao {
    No* raiz;
};

typedef struct {
    const char* p;     /* posição atual no texto */
    int profundidade;  /* aninhamento atual */
    int tem_erro;
    char* erro;
    size_t erro_tam;
} Parser;

/*
 * Uma linha por nome aceito. O campo 'ajuda' alimenta a tela de ajuda do
 * jogo: linhas de apelido (sin, tg, raiz) têm ajuda vazia e não aparecem lá.
 * Para criar uma função nova: acrescente a linha aqui, o caso em
 * avaliar_no() e o valor no enum Funcao. A ajuda se atualiza sozinha.
 */
typedef struct {
    const char* nome;
    Funcao funcao;
    InfoAjuda ajuda;
} EntradaFuncao;

#define APELIDO { NULL, NULL, NULL, 0 }

static const EntradaFuncao TABELA_FUNCOES[] = {
    { "sen",  F_SIN,{ "sen",  "sen(x)",  "seno, angulo em radianos (tambem: sin)", 1 } },
    { "sin",  F_SIN,  APELIDO },
    { "cos",  F_COS,{ "cos",  "cos(x)",  "cosseno, angulo em radianos", 1 } },
    { "tan",  F_TAN,{ "tan",  "tan(x)",  "tangente, dispara perto de 1,57 (tambem: tg)", 1 } },
    { "tg",   F_TAN,  APELIDO },
    { "asin", F_ASIN,{ "asin", "asin(x)", "arco seno, x precisa estar entre -1 e 1", 1 } },
    { "acos", F_ACOS,{ "acos", "acos(x)", "arco cosseno, x precisa estar entre -1 e 1", 1 } },
    { "atan", F_ATAN,{ "atan", "atan(x)", "arco tangente, sobe e se estabiliza", 1 } },
    { "sqrt", F_SQRT,{ "sqrt", "sqrt(x)", "raiz quadrada, x precisa ser 0 ou mais (tambem: raiz)", 1 } },
    { "raiz", F_SQRT, APELIDO },
    { "abs",  F_ABS,{ "abs",  "abs(x)",  "valor absoluto, nunca fica negativo", 1 } },
    { "ln",   F_LN,{ "ln",   "ln(x)",   "logaritmo natural, x precisa ser maior que 0", 1 } },
    { "log",  F_LOG,{ "log",  "log(x)",  "logaritmo na base 10, x precisa ser maior que 0", 1 } },
    { "exp",  F_EXP,{ "exp",  "exp(x)",  "e elevado a x, cresce muito rapido", 1 } }
};

/* Itens de ajuda que não são funções: variável, operadores e constantes. */
static const InfoAjuda TABELA_SIMBOLOS[] = {
    { "x",     "x",       "o quanto o carro ja avancou na pista", 0 },
    { "+  -",  "x+3",     "somar e subtrair", 0 },
    { "*  /",  "2*x/3",   "multiplicar e dividir", 0 },
    { "2x",    "2x+1",    "o * pode ser omitido: 2x vale 2*x", 0 },
    { "^",     "x^2",     "potencia", 0 },
    { "( )",   "(x+1)*2", "parenteses agrupam as contas", 0 },
    { "pi  e", "pi*x",    "constantes 3,14159 e 2,71828", 0 }
};

#define NUM_SIMBOLOS ((int)(sizeof(TABELA_SIMBOLOS) / sizeof(TABELA_SIMBOLOS[0])))
#define NUM_ENTRADAS ((int)(sizeof(TABELA_FUNCOES) / sizeof(TABELA_FUNCOES[0])))

/* ------------------------------------------------------------------ */
/* Utilitários                                                          */
/* ------------------------------------------------------------------ */

static void liberar_no(No* n)
{
    if (!n) return;
    liberar_no(n->esq);
    liberar_no(n->dir);
    free(n);
}

/* Cria um nó. Se faltar memória, libera os filhos e devolve NULL. */
static No* novo_no(TipoNo tipo, No* esq, No* dir)
{
    No* n = (No*)calloc(1, sizeof(No));
    if (!n) {
        liberar_no(esq);
        liberar_no(dir);
        return NULL;
    }
    n->tipo = tipo;
    n->esq = esq;
    n->dir = dir;
    return n;
}

/* Guarda só o primeiro erro encontrado. */
static void definir_erro(Parser* ps, const char* fmt, ...)
{
    va_list args;

    if (ps->tem_erro) return;
    ps->tem_erro = 1;

    if (ps->erro && ps->erro_tam > 0) {
        va_start(args, fmt);
        vsnprintf(ps->erro, ps->erro_tam, fmt, args);
        va_end(args);
    }
}

static void pular_espacos(Parser* ps)
{
    while (isspace((unsigned char)*ps->p)) ps->p++;
}

/* Controla o aninhamento para não estourar a pilha com "((((((...". */
static int entrar(Parser* ps)
{
    if (++ps->profundidade > MAX_PROFUNDIDADE) {
        definir_erro(ps, "expressao complicada demais");
        return 0;
    }
    return 1;
}

static int buscar_funcao(const char* nome, Funcao* saida)
{
    size_t i;
    for (i = 0; i < sizeof(TABELA_FUNCOES) / sizeof(TABELA_FUNCOES[0]); i++) {
        if (strcmp(TABELA_FUNCOES[i].nome, nome) == 0) {
            *saida = TABELA_FUNCOES[i].funcao;
            return 1;
        }
    }
    return 0;
}

/* Multiplicação implícita: "2x", "3sen(x)", "2(x+1)", "x(x+1)". */
static int inicia_implicito(char c)
{
    return isalpha((unsigned char)c) || c == '(';
}

/* ------------------------------------------------------------------ */
/* Parser (descida recursiva)                                           */
/* ------------------------------------------------------------------ */

static No* parse_expr(Parser* ps);

static No* parse_numero(Parser* ps)
{
    double inteiro = 0.0, fracao = 0.0, divisor = 1.0;
    int digitos = 0;
    No* n;

    while (isdigit((unsigned char)*ps->p)) {
        inteiro = inteiro * 10.0 + (*ps->p - '0');
        ps->p++;
        digitos++;
    }
    /* aceita ponto ou virgula como separador decimal */
    if (*ps->p == '.' || *ps->p == ',') {
        ps->p++;
        while (isdigit((unsigned char)*ps->p)) {
            fracao = fracao * 10.0 + (*ps->p - '0');
            divisor *= 10.0;
            ps->p++;
            digitos++;
        }
    }
    if (digitos == 0) {
        definir_erro(ps, "numero invalido");
        return NULL;
    }

    n = novo_no(NO_NUM, NULL, NULL);
    if (!n) return NULL;
    n->valor = inteiro + fracao / divisor;
    return n;
}

static No* parse_identificador(Parser* ps)
{
    char nome[MAX_IDENT + 1];
    size_t len = 0;
    Funcao f;
    No* n, * arg;

    while (isalpha((unsigned char)*ps->p)) {
        if (len < MAX_IDENT) nome[len] = (char)tolower((unsigned char)*ps->p);
        len++;
        ps->p++;
    }
    if (len > MAX_IDENT) {
        definir_erro(ps, "nome muito longo");
        return NULL;
    }
    nome[len] = '\0';

    if (strcmp(nome, "x") == 0) {
        return novo_no(NO_VAR, NULL, NULL);
    }
    if (strcmp(nome, "pi") == 0) {
        n = novo_no(NO_NUM, NULL, NULL);
        if (n) n->valor = VALOR_PI;
        return n;
    }
    if (strcmp(nome, "e") == 0) {
        n = novo_no(NO_NUM, NULL, NULL);
        if (n) n->valor = VALOR_E;
        return n;
    }

    if (buscar_funcao(nome, &f)) {
        pular_espacos(ps);
        if (*ps->p != '(') {
            definir_erro(ps, "falta ( depois de %s", nome);
            return NULL;
        }
        ps->p++;

        if (!entrar(ps)) return NULL;
        arg = parse_expr(ps);
        ps->profundidade--;
        if (!arg) return NULL;

        pular_espacos(ps);
        if (*ps->p != ')') {
            liberar_no(arg);
            definir_erro(ps, "falta fechar parenteses");
            return NULL;
        }
        ps->p++;

        n = novo_no(NO_FUNC, arg, NULL);
        if (!n) return NULL;
        n->funcao = f;
        return n;
    }

    definir_erro(ps, "nao conheco '%s'", nome);
    return NULL;
}

static No* parse_primario(Parser* ps)
{
    char c;

    pular_espacos(ps);
    c = *ps->p;

    if (isdigit((unsigned char)c) || c == '.') {
        return parse_numero(ps);
    }

    if (c == '(') {
        No* dentro;

        ps->p++;
        if (!entrar(ps)) return NULL;
        dentro = parse_expr(ps);
        ps->profundidade--;
        if (!dentro) return NULL;

        pular_espacos(ps);
        if (*ps->p != ')') {
            liberar_no(dentro);
            definir_erro(ps, "falta fechar parenteses");
            return NULL;
        }
        ps->p++;
        return dentro;
    }

    if (isalpha((unsigned char)c)) {
        return parse_identificador(ps);
    }

    if (c == '\0') {
        definir_erro(ps, "a expressao terminou antes da hora");
    }
    else {
        definir_erro(ps, "nao entendi '%c'", c);
    }
    return NULL;
}

static No* parse_unario(Parser* ps);

static No* parse_potencia(Parser* ps)
{
    No* base, * expoente, * n;

    base = parse_primario(ps);
    if (!base) return NULL;

    pular_espacos(ps);
    if (*ps->p != '^') return base;
    ps->p++;

    if (!entrar(ps)) {
        liberar_no(base);
        return NULL;
    }
    expoente = parse_unario(ps);   /* permite 2^-1 e associa à direita */
    ps->profundidade--;
    if (!expoente) {
        liberar_no(base);
        return NULL;
    }

    n = novo_no(NO_BIN, base, expoente);
    if (!n) return NULL;
    n->op = '^';
    return n;
}

static No* parse_unario(Parser* ps)
{
    pular_espacos(ps);

    if (*ps->p == '-' || *ps->p == '+') {
        char sinal = *ps->p;
        No* filho;

        ps->p++;
        if (!entrar(ps)) return NULL;
        filho = parse_unario(ps);
        ps->profundidade--;
        if (!filho) return NULL;

        if (sinal == '+') return filho;
        return novo_no(NO_NEG, filho, NULL);   /* -x^2 vale -(x^2) */
    }

    return parse_potencia(ps);
}

static No* parse_termo(Parser* ps)
{
    No* esq = parse_unario(ps);
    if (!esq) return NULL;

    for (;;) {
        char c, op = '*';
        No* dir, * n;

        pular_espacos(ps);
        c = *ps->p;

        if (c == '*' || c == '/') {
            op = c;
            ps->p++;
        }
        else if (!inicia_implicito(c)) {
            break;
        }
        /* caso contrário é multiplicação implícita: 2x, 3sen(x), 2(x+1) */

        dir = parse_unario(ps);
        if (!dir) {
            liberar_no(esq);
            return NULL;
        }
        n = novo_no(NO_BIN, esq, dir);
        if (!n) return NULL;
        n->op = op;
        esq = n;
    }
    return esq;
}

static No* parse_expr(Parser* ps)
{
    No* esq = parse_termo(ps);
    if (!esq) return NULL;

    for (;;) {
        char op;
        No* dir, * n;

        pular_espacos(ps);
        op = *ps->p;
        if (op != '+' && op != '-') break;
        ps->p++;

        dir = parse_termo(ps);
        if (!dir) {
            liberar_no(esq);
            return NULL;
        }
        n = novo_no(NO_BIN, esq, dir);
        if (!n) return NULL;
        n->op = op;
        esq = n;
    }
    return esq;
}

/* ------------------------------------------------------------------ */
/* Avaliação                                                            */
/* ------------------------------------------------------------------ */

static double avaliar_no(const No* n, double x)
{
    double a, b;

    switch (n->tipo) {
    case NO_NUM:
        return n->valor;
    case NO_VAR:
        return x;
    case NO_NEG:
        return -avaliar_no(n->esq, x);
    case NO_BIN:
        a = avaliar_no(n->esq, x);
        b = avaliar_no(n->dir, x);
        switch (n->op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;     /* divisão por zero dá infinito/NaN */
        case '^': return pow(a, b);
        default:  break;
        }
        break;
    case NO_FUNC:
        a = avaliar_no(n->esq, x);
        switch (n->funcao) {
        case F_SIN:  return sin(a);
        case F_COS:  return cos(a);
        case F_TAN:  return tan(a);
        case F_ASIN: return asin(a);
        case F_ACOS: return acos(a);
        case F_ATAN: return atan(a);
        case F_SQRT: return sqrt(a);
        case F_ABS:  return fabs(a);
        case F_LN:   return log(a);
        case F_LOG:  return log10(a);
        case F_EXP:  return exp(a);
        }
        break;
    }
    return (double)NAN;
}

/* ------------------------------------------------------------------ */
/* API pública                                                          */
/* ------------------------------------------------------------------ */

Expressao* expressao_compilar(const char* texto, char* erro, size_t erro_tam)
{
    Parser ps;
    Expressao* e;
    No* raiz;

    if (erro && erro_tam > 0) erro[0] = '\0';

    ps.p = texto ? texto : "";
    ps.profundidade = 0;
    ps.tem_erro = 0;
    ps.erro = erro;
    ps.erro_tam = erro_tam;

    if (strlen(ps.p) > MAX_TEXTO) {
        definir_erro(&ps, "texto muito longo");
        return NULL;
    }

    pular_espacos(&ps);
    if (*ps.p == '\0') {
        definir_erro(&ps, "digite uma funcao, por exemplo: sen(x)");
        return NULL;
    }

    raiz = parse_expr(&ps);
    if (raiz) {
        pular_espacos(&ps);
        if (*ps.p != '\0') {
            if (*ps.p == ')') {
                definir_erro(&ps, "parenteses fechando sem abrir");
            }
            else {
                definir_erro(&ps, "nao entendi '%c' (faltou um operador?)", *ps.p);
            }
            liberar_no(raiz);
            return NULL;
        }
    }
    if (!raiz) {
        definir_erro(&ps, "sem memoria");   /* so vale se nenhum erro anterior */
        return NULL;
    }

    e = (Expressao*)malloc(sizeof(Expressao));
    if (!e) {
        liberar_no(raiz);
        definir_erro(&ps, "sem memoria");
        return NULL;
    }
    e->raiz = raiz;
    return e;
}

double expressao_avaliar(const Expressao* e, double x)
{
    if (!e || !e->raiz) return (double)NAN;
    return avaliar_no(e->raiz, x);
}

void expressao_liberar(Expressao* e)
{
    if (!e) return;
    liberar_no(e->raiz);
    free(e);
}

/* ------------------------------------------------------------------ */
/* Ajuda (lista de tudo que o jogador pode digitar)                     */
/* ------------------------------------------------------------------ */

int expressao_num_ajuda(void)
{
    int i, funcoes = 0;

    for (i = 0; i < NUM_ENTRADAS; i++) {
        if (TABELA_FUNCOES[i].ajuda.descricao) funcoes++;
    }
    return NUM_SIMBOLOS + funcoes;
}

const InfoAjuda* expressao_ajuda(int indice)
{
    int i, contador;

    if (indice < 0) return NULL;
    if (indice < NUM_SIMBOLOS) return &TABELA_SIMBOLOS[indice];

    contador = indice - NUM_SIMBOLOS;
    for (i = 0; i < NUM_ENTRADAS; i++) {
        if (!TABELA_FUNCOES[i].ajuda.descricao) continue;   /* apelido */
        if (contador == 0) return &TABELA_FUNCOES[i].ajuda;
        contador--;
    }
    return NULL;
}