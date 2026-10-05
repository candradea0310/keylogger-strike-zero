#pragma once
#include <windows.h>
#include <string>

bool DiscordEnviarTexto(const std::wstring& webhook_url, const std::wstring& mensaje);
bool DiscordEnviarArchivo(const std::wstring& webhook_url,
                          const std::wstring& ruta_archivo,
                          const std::wstring& comentario);

std::wstring Utf8ToWide(const std::string& str);
std::string  WideToUtf8(const std::wstring& wstr);