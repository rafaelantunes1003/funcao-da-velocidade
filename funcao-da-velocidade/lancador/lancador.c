/*
 * jogo.exe: fica na pasta principal do repositório e só abre o jogo de verdade,
 * que está em Jogo/ junto com as DLLs e a pasta images.
 * Não usa Allegro nem DLL nenhuma, por isso pode ficar sozinho na pasta principal.
 */
#include <windows.h>
#include <wchar.h>

#define JOGO_PASTA L"Jogo"
#define JOGO_EXE   L"Funcao da Velocidade.exe"

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE anterior, PWSTR linha, int mostrar)
{
    (void)inst; (void)anterior; (void)linha; (void)mostrar;

    /* Pasta onde está este jogo.exe */
    wchar_t pasta[MAX_PATH];
    DWORD n = GetModuleFileNameW(NULL, pasta, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return 1;
    wchar_t *barra = wcsrchr(pasta, L'\\');
    if (barra) *barra = L'\0';

    wchar_t pasta_jogo[MAX_PATH];
    wchar_t exe[MAX_PATH];
    wchar_t comando[MAX_PATH + 2];
    _snwprintf(pasta_jogo, MAX_PATH, L"%ls\\%ls", pasta, JOGO_PASTA);
    _snwprintf(exe, MAX_PATH, L"%ls\\%ls", pasta_jogo, JOGO_EXE);
    _snwprintf(comando, MAX_PATH + 2, L"\"%ls\"", exe);
    pasta_jogo[MAX_PATH - 1] = exe[MAX_PATH - 1] = comando[MAX_PATH + 1] = L'\0';

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessW(exe, comando, NULL, NULL, FALSE, 0, NULL, pasta_jogo, &si, &pi)) {
        MessageBoxW(NULL,
            L"Nao encontrei a pasta Jogo.\n"
            L"Extraia o repositorio inteiro e deixe o jogo.exe ao lado da pasta Jogo.",
            L"Funcao da Velocidade", MB_OK | MB_ICONERROR);
        return 1;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
