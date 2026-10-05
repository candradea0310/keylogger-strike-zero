#include "cripto.h"

std::wstring DescifrarString(const unsigned char* datos, size_t tamano, unsigned char clave) {
    std::wstring resultado;
    for (size_t i = 0; i + 1 < tamano; i += 2) {
        wchar_t c = (wchar_t)(datos[i] | (datos[i+1] << 8));
        c ^= (wchar_t)clave;
        resultado.push_back(c);
    }
    return resultado;
}

std::wstring CifrarString(const std::wstring& entrada, unsigned char clave) {
    std::wstring resultado;
    for (wchar_t c : entrada) {
        resultado.push_back(c ^ (wchar_t)clave);
    }
    return resultado;
}