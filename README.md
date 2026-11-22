# funcao-da-velocidade

Jogo em C com Allegro 5: o jogador digita uma função de x (ex.: `sen(x)`, `x/2`, `2sen(x)+1`) e o carro anda em cima do gráfico dela. Para vencer, o carro precisa chegar na faixa quadriculada sem sair do asfalto e sem bater nos obstáculos.

## Como jogar

1. Na tela inicial, clique em **PLAY**.
2. Olhe o formato da estrada, digite uma função de x e aperte **ENTER**.
3. Se bater ou sair da pista, corrija a função e aperte **ENTER** de novo.
4. Ao vencer, aperte **ENTER** para ir para a próxima fase.

## Fases

| Fase | Pista |
|------|-------|
| 1 | reta |
| 2 | subida em linha reta |
| 3 | parábola em forma de U |

## Como compilar

**Visual Studio:** abrir `funcao-da-velocidade.slnx` e rodar (o Allegro vem pelo NuGet).

**gcc (MSYS2 UCRT64):** dentro da pasta `funcao-da-velocidade/`:

```
gcc -Wall src/*.c -o movimento.exe $(pkg-config --cflags --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5) -lm
./movimento.exe
```

## Organização

```
funcao-da-velocidade/
  src/
    main.c            inicia o Allegro e roda o laço principal
    config.h          tamanhos da tela, da pista e velocidade
    menu.c/.h         tela inicial com o botão PLAY
    fases.c/.h        pista, linha do meio e obstáculos de cada fase
    jogo.c/.h         estado do carro (parado, andando, chegou, saiu, colidiu)
    obstaculos.c/.h   imagens e colisão dos obstáculos
    desenho.c/.h      pista, gráfico, carro e mensagens
    campo_texto.c/.h  caixa onde se digita a função
    coordenadas.c/.h  converte (x, y) do gráfico para pixels
    expressao.c/.h    interpretador das funções digitadas
  testes/
    teste_expressao.c  testes do interpretador
  images/
    capa.png, carro.png, pista1.png a pista3.png, pedra.png, cone.png, buraco.png
```
