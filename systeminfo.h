#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct InfoRed {
    std::wstring nombre;
    std::wstring ip;
    std::wstring mascara;
    std::wstring mac;
    std::wstring tipo;
};

std::vector<InfoRed> ObtenerInfoRed();
std::wstring ObtenerIPPublica();
std::wstring ObtenerRAM();
std::wstring ObtenerArquitectura();
std::vector<std::wstring> ObtenerProcesosActivos(int maximo);
std::wstring ObtenerUptime();
void EnviarInfoSistemaCompleta();