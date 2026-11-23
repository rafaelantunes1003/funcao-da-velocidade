$ErrorActionPreference = "Stop"

$repoRoot = $PSScriptRoot
$solution = Join-Path $repoRoot "funcao-da-velocidade.slnx"
$projectDir = Join-Path $repoRoot "funcao-da-velocidade"
$packagesDir = Join-Path $repoRoot "packages"
$distDir = Join-Path $repoRoot "dist\windows-x64"

if (-not (Test-Path $solution)) {
    throw "Solucao nao encontrada: $solution"
}

$msbuild = Get-Command msbuild.exe -ErrorAction SilentlyContinue
if (-not $msbuild) {
    throw "msbuild.exe nao encontrado. Abra o Developer PowerShell for Visual Studio e execute este script novamente."
}

Write-Host "Compilando Release x64..."
& $msbuild.Source $solution /m /t:Build /p:Configuration=Release /p:Platform=x64 /p:RestorePackagesConfig=true
if ($LASTEXITCODE -ne 0) {
    throw "A compilacao falhou. Confira as mensagens do MSBuild e restaure os pacotes NuGet no Visual Studio."
}

$exe = Get-ChildItem -Path $repoRoot -Filter "*.exe" -File -Recurse |
    Where-Object { $_.FullName -notmatch '[\\/](dist|packages)[\\/]' -and $_.FullName -match '[\\/]x64[\\/]Release[\\/]' } |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $exe) {
    throw "Executavel nao encontrado na saida Release x64."
}

if (Test-Path $distDir) {
    Remove-Item $distDir -Recurse -Force
}
New-Item -ItemType Directory -Path $distDir -Force | Out-Null

Copy-Item $exe.FullName (Join-Path $distDir $exe.Name) -Force
Copy-Item (Join-Path $projectDir "images") (Join-Path $distDir "images") -Recurse -Force

if (-not (Test-Path $packagesDir)) {
    throw "Pasta packages nao encontrada. Restaure os pacotes NuGet primeiro."
}

$dlls = Get-ChildItem -Path $packagesDir -Filter "*.dll" -File -Recurse |
    Where-Object {
        $_.FullName -match '(?i)[\\/](x64|amd64)[\\/]' -and
        $_.FullName -notmatch '(?i)[\\/](x86|win32)[\\/]'
    }

if (-not $dlls) {
    throw "Nenhuma DLL x64 foi encontrada nos pacotes NuGet. Verifique a estrutura de Allegro e AllegroDeps antes de distribuir."
}

foreach ($dll in $dlls) {
    Copy-Item $dll.FullName $distDir -Force
}

Write-Host ""
Write-Host "Distribuicao criada em: $distDir"
Write-Host "Inclui o executavel, imagens e DLLs x64 encontradas nos pacotes NuGet."
Write-Host "Teste esta pasta em um Windows limpo antes de publicar."
