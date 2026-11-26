# Monta a pasta Jogo/ com o executavel pronto para jogar (exe + DLLs + images)
# e o jogo.exe da pasta principal, que so abre o jogo que esta em Jogo/.
# Precisa do MSYS2 UCRT64 com gcc, pkgconf e allegro instalados.
# Uso: .\gerar-jogo.ps1              (MSYS2 em C:\msys64)
#      .\gerar-jogo.ps1 -Msys D:\msys64
param([string]$Msys = "C:\msys64")

$ErrorActionPreference = "Stop"

$raiz    = $PSScriptRoot
$projeto = Join-Path $raiz "funcao-da-velocidade"
$destino = Join-Path $raiz "Jogo"
$bin     = Join-Path $Msys "ucrt64\bin"
$exe     = Join-Path $destino "Funcao da Velocidade.exe"
$atalho  = Join-Path $raiz "jogo.exe"

if (-not (Test-Path (Join-Path $bin "gcc.exe"))) {
    throw "gcc nao encontrado em $bin. Instale o MSYS2 UCRT64 ou informe a pasta com -Msys."
}
$env:PATH = "$bin;$env:PATH"

# Recomeca a pasta do zero para nao sobrar arquivo velho
if (Test-Path $destino) { Remove-Item $destino -Recurse -Force }
New-Item -ItemType Directory $destino | Out-Null

Write-Host "Compilando..."
Push-Location $projeto
try {
    & windres icone/icone.rc -O coff -o icone/icone.o
    if ($LASTEXITCODE -ne 0) { throw "Falha ao compilar o icone." }

    $flags  = (& pkg-config --cflags --libs allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5) -split ' ' | Where-Object { $_ }
    $fontes = Get-ChildItem src\*.c | ForEach-Object { "src/" + $_.Name }
    # -mwindows: abre so a janela do jogo, sem o console preto atras
    & gcc -std=c11 -O2 -Wall -mwindows $fontes icone/icone.o -o $exe @flags -lm
    if ($LASTEXITCODE -ne 0) { throw "A compilacao falhou." }

    # jogo.exe da pasta principal: -static para nao precisar de nenhuma DLL
    & gcc -O2 -Wall -mwindows -municode -static lancador/lancador.c icone/icone.o -o $atalho
    if ($LASTEXITCODE -ne 0) { throw "A compilacao do jogo.exe falhou." }
}
finally {
    Remove-Item icone/icone.o -ErrorAction SilentlyContinue
    Pop-Location
}

Write-Host "Copiando as DLLs que o jogo usa..."
$fila = New-Object System.Collections.Queue
$fila.Enqueue($exe)
$copiadas = @{}
while ($fila.Count -gt 0) {
    $arquivo = $fila.Dequeue()
    $nomes = & objdump -p $arquivo | Select-String 'DLL Name: (.+)$' |
        ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() }
    foreach ($nome in $nomes) {
        $origem = Join-Path $bin $nome
        # So copia DLLs do MSYS2; as do Windows (kernel32, user32...) ja existem em todo PC
        if ((Test-Path $origem) -and -not $copiadas.ContainsKey($nome.ToLower())) {
            Copy-Item $origem $destino
            $copiadas[$nome.ToLower()] = $true
            $fila.Enqueue($origem)
        }
    }
}

Write-Host "Copiando as imagens..."
Copy-Item (Join-Path $projeto "images") (Join-Path $destino "images") -Recurse

Write-Host ""
Write-Host "Pronto: $destino"
Write-Host "$($copiadas.Count) DLLs copiadas. Para jogar, abra o jogo.exe da pasta principal."
