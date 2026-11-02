/*
 * esse arquivo é so pra testar e ver se tá funcionando
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "expressao.h"

#define PI 3.14159265358979323846

static int total = 0;
static int falhas = 0;

typedef struct {
    const char* texto;
    double x;
    double esperado;
} CasoValor;

/* Expressões válidas e o valor esperado para um dado x */
static const CasoValor CASOS_VALOR[] = {
    { "2*x+1",             3.0,     7.0 },
    { "2x+1",              3.0,     7.0 },
    { "x^2",               4.0,    16.0 },
    { "-x^2",              3.0,    -9.0 },
    { "2^3^2",             0.0,   512.0 },
    { "2^-1",              0.0,     0.5 },
    { "sen(x)",           PI / 2,   1.0 },
    { "sin(x)",           PI / 2,   1.0 },
    { "SEN(X)",           PI / 2,   1.0 },
    { "3sen(x)",          PI / 2,   3.0 },
    { "2(x+1)",            2.0,     6.0 },
    { "(x+1)(x-1)",        3.0,     8.0 },
    { "x(x+1)",            2.0,     6.0 },
    { "sqrt(x)",           9.0,     3.0 },
    { "raiz(x)+abs(-2)",  16.0,     6.0 },
    { "10/4",              0.0,     2.5 },
    { "0,5*x",             4.0,     2.0 },
    { "0.1+0.2",           0.0,     0.3 },
    { "pi*x",              2.0,  2 * PI },
    { "e^x",               0.0,     1.0 },
    { "ln(e)",             0.0,     1.0 },
    { "log(1000)",         0.0,     3.0 },
    { "  2 * ( x + 1 )  ", 1.0,     4.0 },
    { "2x^2",              3.0,    18.0 },
    { "--x",               5.0,     5.0 },
    { "+x",                2.0,     2.0 },
    { "x*-1",              3.0,    -3.0 },
    { "5-3-1",             0.0,     1.0 },
    { "8/4/2",             0.0,     1.0 },
    { "cos(0)",            0.0,     1.0 },
    { "x^2-4x+3",          1.0,     0.0 },
    { "3sen(2x)+1",       PI / 4,   4.0 }
};

/* Expressões válidas que dão resultado inválido (NaN ou infinito) */
typedef struct {
    const char* texto;
    double x;
} CasoInvalido;

static const CasoInvalido CASOS_NAO_FINITOS[] = {
    { "1/x",     0.0 },
    { "sqrt(x)", -1.0 },
    { "ln(x)",    0.0 },
    { "ln(x)",   -2.0 }
};

/* Textos que NÃO devem compilar */
static const char* CASOS_ERRO[] = {
    "", "   ", "2+", "(2+3", "2+3)", "sen x", "foo(x)", "2 3",
    "x**2", "*2", "sen()", "2..5", "x y", "sen(x", "^2", "2x3", "@"
};

static void verificar(int ok, const char* descricao)
{
    total++;
    if (!ok) {
        falhas++;
        printf("FALHOU: %s\n", descricao);
    }
}

static void testar_valores(void)
{
    size_t i;
    char erro[128];
    char desc[200];

    for (i = 0; i < sizeof(CASOS_VALOR) / sizeof(CASOS_VALOR[0]); i++) {
        const CasoValor* c = &CASOS_VALOR[i];
        Expressao* e = expressao_compilar(c->texto, erro, sizeof(erro));

        if (!e) {
            snprintf(desc, sizeof(desc), "\"%s\" deveria compilar (erro: %s)", c->texto, erro);
            verificar(0, desc);
            continue;
        }
        {
            double r = expressao_avaliar(e, c->x);
            snprintf(desc, sizeof(desc), "\"%s\" com x=%g deu %g, esperado %g",
                c->texto, c->x, r, c->esperado);
            verificar(fabs(r - c->esperado) < 1e-9, desc);
        }
        expressao_liberar(e);
    }
}

static void testar_nao_finitos(void)
{
    size_t i;
    char erro[128];
    char desc[200];

    for (i = 0; i < sizeof(CASOS_NAO_FINITOS) / sizeof(CASOS_NAO_FINITOS[0]); i++) {
        const CasoInvalido* c = &CASOS_NAO_FINITOS[i];
        Expressao* e = expressao_compilar(c->texto, erro, sizeof(erro));

        if (!e) {
            snprintf(desc, sizeof(desc), "\"%s\" deveria compilar", c->texto);
            verificar(0, desc);
            continue;
        }
        snprintf(desc, sizeof(desc), "\"%s\" com x=%g deveria ser nao finito", c->texto, c->x);
        verificar(!isfinite(expressao_avaliar(e, c->x)), desc);
        expressao_liberar(e);
    }
}

static void testar_erros(void)
{
    size_t i;
    char erro[128];
    char desc[200];

    printf("\nMensagens de erro:\n");
    for (i = 0; i < sizeof(CASOS_ERRO) / sizeof(CASOS_ERRO[0]); i++) {
        Expressao* e = expressao_compilar(CASOS_ERRO[i], erro, sizeof(erro));

        snprintf(desc, sizeof(desc), "\"%s\" nao deveria compilar", CASOS_ERRO[i]);
        verificar(e == NULL, desc);
        if (e) {
            expressao_liberar(e);
        }
        else {
            printf("  \"%s\" -> %s\n", CASOS_ERRO[i], erro);
        }
    }
}

static void testar_aninhamento(void)
{
    char texto[400];
    char erro[128];
    Expressao* e;
    int i, n;

    /* 30 parênteses aninhados: deve funcionar */
    n = 0;
    for (i = 0; i < 30; i++) texto[n++] = '(';
    texto[n++] = 'x';
    for (i = 0; i < 30; i++) texto[n++] = ')';
    texto[n] = '\0';
    e = expressao_compilar(texto, erro, sizeof(erro));
    verificar(e != NULL, "30 parenteses aninhados deveriam compilar");
    if (e) {
        verificar(fabs(expressao_avaliar(e, 7.0) - 7.0) < 1e-9, "30 parenteses devolvem x");
        expressao_liberar(e);
    }

    /* 100 parênteses aninhados: deve recusar sem travar */
    n = 0;
    for (i = 0; i < 100; i++) texto[n++] = '(';
    texto[n++] = 'x';
    for (i = 0; i < 100; i++) texto[n++] = ')';
    texto[n] = '\0';
    e = expressao_compilar(texto, erro, sizeof(erro));
    verificar(e == NULL, "100 parenteses aninhados deveriam ser recusados");
    expressao_liberar(e);

    /* texto gigante: deve recusar */
    memset(texto, 'x', 300);
    texto[300] = '\0';
    e = expressao_compilar(texto, erro, sizeof(erro));
    verificar(e == NULL, "texto de 300 caracteres deveria ser recusado");
    expressao_liberar(e);
}

/* Todo exemplo da ajuda precisa compilar e dar um valor valido em x = 0.5 */
static void testar_ajuda(void)
{
    int i, n = expressao_num_ajuda();
    char erro[128];
    char desc[200];

    verificar(n > 0, "a ajuda deveria ter itens");
    verificar(expressao_ajuda(-1) == NULL && expressao_ajuda(n) == NULL,
        "indices fora do intervalo deveriam dar NULL");

    printf("\nAjuda do jogo (%d itens):\n", n);
    for (i = 0; i < n; i++) {
        const InfoAjuda* a = expressao_ajuda(i);
        Expressao* e;

        snprintf(desc, sizeof(desc), "item %d da ajuda esta incompleto", i);
        verificar(a && a->titulo && a->exemplo && a->descricao, desc);
        if (!a || !a->titulo || !a->exemplo || !a->descricao) continue;

        e = expressao_compilar(a->exemplo, erro, sizeof(erro));
        snprintf(desc, sizeof(desc), "exemplo \"%s\" da ajuda deveria compilar", a->exemplo);
        verificar(e != NULL, desc);
        if (e) {
            snprintf(desc, sizeof(desc), "exemplo \"%s\" deveria dar valor valido em x=0.5", a->exemplo);
            verificar(isfinite(expressao_avaliar(e, 0.5)), desc);
            expressao_liberar(e);
        }
        printf("  %-7s %-9s %s\n", a->titulo, a->exemplo, a->descricao);
    }
}

static void modo_interativo(void)
{
    char linha[300];
    char erro[128];

    printf("\nDigite uma funcao de x (linha vazia para sair).\n");
    for (;;) {
        Expressao* e;
        int i;
        size_t len;

        printf("f(x) = ");
        if (!fgets(linha, sizeof(linha), stdin)) break;

        len = strlen(linha);
        while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r')) {
            linha[--len] = '\0';
        }
        if (len == 0) break;

        e = expressao_compilar(linha, erro, sizeof(erro));
        if (!e) {
            printf("  erro: %s\n", erro);
            continue;
        }
        for (i = -3; i <= 3; i++) {
            double y = expressao_avaliar(e, (double)i);
            if (isfinite(y)) {
                printf("  x = %2d  ->  y = %.4f\n", i, y);
            }
            else {
                printf("  x = %2d  ->  indefinido\n", i);
            }
        }
        expressao_liberar(e);
    }
}

int main(void)
{
    testar_valores();
    testar_nao_finitos();
    testar_erros();
    testar_aninhamento();
    testar_ajuda();

    printf("\n%d verificacoes, %d falha(s).\n", total, falhas);
    if (falhas > 0) return 1;

    modo_interativo();
    return 0;
}