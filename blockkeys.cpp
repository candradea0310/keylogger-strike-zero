#include "blockkeys.h"
#include <windows.h>

static HHOOK g_hookBlock = NULL;

static LRESULT CALLBACK BloqueoProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        // Bloquear Alt+F4
        if (kb->vkCode == VK_F4 &&
            (GetAsyncKeyState(VK_MENU) & 0x8000)) {
            return 1; // Bloquear
        }

        // Bloquear Ctrl+Shift+Esc (Task Manager)
        if (kb->vkCode == VK_ESCAPE &&
            (GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
            (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
            return 1;
        }

        // Bloquear Win+L (lock screen)
        if (kb->vkCode == 'L' &&
            (GetAsyncKeyState(VK_LWIN) & 0x8000)) {
            return 1;
        }
    }
    return CallNextHookEx(g_hookBlock, nCode, wParam, lParam);
}

static DWORD WINAPI HiloBloqueo(LPVOID) {
    g_hookBlock = SetWindowsHookExW(WH_KEYBOARD_LL, BloqueoProc,
        GetModuleHandleW(NULL), 0);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

void IniciarBloqueoTeclas() {
    CreateThread(NULL, 0, HiloBloqueo, NULL, 0, NULL);
}