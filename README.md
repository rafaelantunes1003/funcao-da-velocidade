# funcao-da-velocidade

Jogo em C com Allegro 5: o jogador digita uma função de x (ex.: `sen(x)`, `x/2`, `2sen(x)+1`) e o carro anda em cima do gráfico dela. Se a função sair da pista, o carro para.

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
    main.c          inicia o Allegro e roda o laço principal
    config.h        tamanhos da tela, da pista e velocidade
    jogo.c/.h       estado do carro (parado, andando, chegou, saiu)
    desenho.c/.h    grade, gráfico, carro e mensagens
    campo_texto.c/.h  caixa onde se digita a função
    coordenadas.c/.h  converte (x, y) do gráfico para pixels
    expressao.c/.h  interpretador das funções digitadas
  testes/
    teste_expressao.c  testes do interpretador
  images/
```
