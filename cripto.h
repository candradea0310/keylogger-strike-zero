#pragma once
#include <windows.h>
#include <string>

std::wstring DescifrarString(const unsigned char* datos, size_t tamano, unsigned char clave);
std::wstring CifrarString(const std::wstring& entrada, unsigned char clave);