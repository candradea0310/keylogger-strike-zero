#include "gui.h"
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#define ID_PROGRESS  1001
#define ID_ACEPTAR   1002

static HWND g_hProgress = NULL;
static HWND g_hEstado   = NULL;
static int  g_progreso  = 0;

static VOID CALLBACK TimerProc(HWND hWnd, UINT, UINT_PTR, DWORD) {
    if (g_progreso >= 100) {
        SetWindowTextW(g_hEstado, L"\u2705 Recompensas reclamadas con exito");
        HWND hBoton = GetDlgItem(hWnd, ID_ACEPTAR);
        EnableWindow(hBoton, TRUE);
        SetWindowTextW(hBoton, L"Aceptar");
        return;
    }
    g_progreso += 3;
    if (g_progreso > 100) g_progreso = 100;
    SendMessageW(g_hProgress, PBM_SETPOS, (WPARAM)g_progreso, 0);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindowW(L"STATIC", L"Mejora de Arma - Nivel 2",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                20, 20, 400, 30, hWnd, NULL, NULL, NULL);
            g_hEstado = CreateWindowW(L"STATIC", L"Aplicando mejora...",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                20, 60, 400, 25, hWnd, NULL, NULL, NULL);
            g_hProgress = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
                WS_CHILD | WS_VISIBLE,
                40, 100, 360, 25, hWnd, (HMENU)ID_PROGRESS, NULL, NULL);
            SendMessageW(g_hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            HWND hBoton = CreateWindowW(L"BUTTON", L"Aplicando...",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                160, 150, 120, 35, hWnd, (HMENU)ID_ACEPTAR, NULL, NULL);
            EnableWindow(hBoton, FALSE);
            SetTimer(hWnd, 1, 50, TimerProc);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_ACEPTAR) DestroyWindow(hWnd);
            return 0;
        case WM_CLOSE:
            if (g_progreso >= 100) DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            KillTimer(hWnd, 1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void MostrarVentanaFalsa() {
    HINSTANCE hInst = GetModuleHandleW(NULL);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = L"MejoraArmaWnd";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int ancho = 460, alto = 240;
    int x = (GetSystemMetrics(SM_CXSCREEN) - ancho) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - alto) / 2;

    HWND hWnd = CreateWindowExW(0, L"MejoraArmaWnd",
        L"Mejora de Arma - Nivel 2",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, ancho, alto, NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}