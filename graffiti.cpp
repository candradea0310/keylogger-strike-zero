#include "graffiti.h"
#include <windows.h>
#include <string>

void CambiarFondoPantalla() {
    // Crear una imagen BMP negra simple con texto
    // Por simplicidad, usamos un color sólido

    const int ancho = 1920, alto = 1080;
    const int bytes = ancho * alto * 3;

    BITMAPFILEHEADER bfh = {0};
    BITMAPINFOHEADER bih = {0};

    bfh.bfType = 0x4D42;
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bytes;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = ancho;
    bih.biHeight = alto;
    bih.biPlanes = 1;
    bih.biBitCount = 24;

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring ruta = std::wstring(tempPath) + L"wallpaper.bmp";

    HANDLE hFile = CreateFileW(ruta.c_str(), GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &bfh, sizeof(bfh), &written, NULL);
        WriteFile(hFile, &bih, sizeof(bih), &written, NULL);

        // Rellenar con color rojo oscuro + patrón
        unsigned char* datos = new unsigned char[bytes];
        for (int i = 0; i < bytes; i += 3) {
            datos[i] = 0x10;      // B
            datos[i+1] = 0x10;    // G
            datos[i+2] = 0x80;    // R (rojo oscuro)
        }
        WriteFile(hFile, datos, bytes, &written, NULL);
        delete[] datos;
        CloseHandle(hFile);

        SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)ruta.c_str(),
            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    }
}