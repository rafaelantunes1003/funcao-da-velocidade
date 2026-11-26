# funcao-da-velocidade

Jogo em C com Allegro 5: o jogador digita uma funcao de x (ex.: `sen(x)`, `x/2`, `2sen(x)+1`) e o carro anda em cima do grafico dela. Para vencer, o carro precisa chegar a faixa quadriculada sem sair do asfalto e sem bater nos obstaculos.

## Jogar sem instalar nada (Windows 10 ou 11)

1. Baixe o repositorio: botao verde **Code > Download ZIP** e extraia o ZIP.
2. Abra a pasta `Jogo` e de dois cliques em **Funcao da Velocidade.exe** (o icone do carrinho vermelho).

Se aparecer "O Windows protegeu o computador", clique em **Mais informacoes > Executar assim mesmo**. O aviso aparece porque o executavel nao tem assinatura digital.

Nao tire o executavel de dentro da pasta `Jogo`: ele precisa das DLLs e da pasta `images` ao lado dele.

## Como jogar

1. Na tela inicial, clique em **PLAY**.
2. Olhe o formato da estrada, digite uma funcao de x e aperte **ENTER**.
3. Se bater ou sair da pista, corrija a funcao e aperte **ENTER** de novo.
4. Ao vencer, aperte **ENTER** para ir para a proxima fase.

## Fases

| Fase | Pista |
|------|-------|
| 1 | reta |
| 2 | subida em linha reta |
| 3 | parabola em forma de U |

## Compilar no Visual Studio (Windows)

### Requisitos

- Windows 10 ou 11 de 64 bits.
- Visual Studio com a carga de trabalho **Desenvolvimento para desktop com C++**.
- Componente **MSVC** e Windows SDK instalados.
- NuGet habilitado no Visual Studio.

### Passos

1. Baixe ou clone este repositorio mantendo a estrutura de pastas.
2. Abra `funcao-da-velocidade.slnx` no Visual Studio.
3. Se o Visual Studio perguntar, permita a restauracao dos pacotes NuGet. Tambem e possivel clicar com o botao direito na solucao e escolher **Restaurar Pacotes NuGet**.
4. Selecione **Release** e **x64** na barra superior.
5. Escolha **Compilar > Compilar Solucao**.

As dependencias nativas Allegro 5.2.11.3 e AllegroDeps 1.16.0 estao declaradas em `funcao-da-velocidade/packages.config`. Nao e necessario configurar manualmente os diretorios de include e lib quando a restauracao NuGet funcionar.

Para desenvolvimento, tambem e possivel selecionar **Debug | x64**.

## Criar a pasta para distribuir o jogo

Depois de restaurar os pacotes NuGet, abra o **Developer PowerShell for Visual Studio** na pasta raiz do repositorio e execute:

```powershell
.\empacotar-windows.ps1
```

O script compila em **Release x64** e tenta montar a pasta `dist/windows-x64` com:

- o executavel do jogo;
- a pasta `images` com os recursos visuais;
- as DLLs x64 encontradas nos pacotes NuGet do Allegro.

Distribua a pasta `dist/windows-x64` inteira, sem separar o executavel das imagens e DLLs. O computador de destino nao precisa do Allegro nem do Visual Studio instalados. Pode ser necessario instalar o **Microsoft Visual C++ Redistributable x64**, conforme as dependencias do compilador.

**Importante:** o script verifica se encontrou DLLs x64 e interrompe o empacotamento se nao encontrar. Antes de publicar, teste a pasta em outro Windows que nao tenha Allegro instalado. A criacao do pacote nao substitui esse teste.

## Compilar com GCC (MSYS2 UCRT64)

Dentro da pasta `funcao-da-velocidade/`:

```bash
gcc -Wall src/*.c -o movimento.exe $(pkg-config --cflags --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5) -lm
./movimento.exe
```

## Atualizar a pasta Jogo

A pasta `Jogo` nao se atualiza sozinha. Depois de mudar o codigo ou as imagens, gere ela de novo (precisa do MSYS2 UCRT64 com gcc, pkgconf e allegro) e envie junto no commit:

```powershell
.\gerar-jogo.ps1                 # MSYS2 em C:\msys64
.\gerar-jogo.ps1 -Msys D:\msys64 # MSYS2 em outra pasta
```

O script compila o jogo sem a janela do console, com o icone de `funcao-da-velocidade/icone`, e copia so as DLLs que o executavel usa, junto com a pasta `images`.

## Organizacao

```
Jogo/                  jogo pronto para abrir (exe + DLLs + images)
funcao-da-velocidade/
  src/                 codigo-fonte em C
  testes/              testes do interpretador
  images/              imagens do jogo
  icone/               icone do executavel
funcao-da-velocidade.slnx
gerar-jogo.ps1         script que monta a pasta Jogo (MSYS2)
empacotar-windows.ps1  script para montar a distribuicao Windows x64 (Visual Studio)
```
