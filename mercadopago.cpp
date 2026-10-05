#include "mercadopago.h"
#include <windows.h>
#include <winhttp.h>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdio>

#pragma comment(lib, "winhttp.lib")

// Función auxiliar para mostrar debug con MessageBox
static void DebugBox(const std::string& mensaje) {
    MessageBoxA(NULL, mensaje.c_str(), "MP DEBUG", MB_OK | MB_ICONINFORMATION);
}

MercadoPagoClient::MercadoPagoClient(const MercadoPagoConfig& config) : config_(config) {}

std::string MercadoPagoClient::HttpPost(const std::string& url, const std::string& payload) {
    std::string response;

    URL_COMPONENTS urlComp = {0};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;

    std::wstring wUrl(url.begin(), url.end());

    if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp)) {
        DebugBox("Error al parsear URL");
        return "";
    }

    std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

    HINTERNET hSession = WinHttpOpen(L"MercadoPagoClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { DebugBox("WinHttpOpen fallo"); return ""; }

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
        urlComp.nPort ? urlComp.nPort : INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { DebugBox("WinHttpConnect fallo"); WinHttpCloseHandle(hSession); return ""; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        DebugBox("WinHttpOpenRequest fallo");
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"Authorization: Bearer ";
    headers += std::wstring(config_.accessToken.begin(), config_.accessToken.end());
    headers += L"\r\n";

    std::wstring wPayload(payload.begin(), payload.end());

    BOOL bResult = WinHttpSendRequest(hRequest,
        headers.c_str(), -1L,
        (LPVOID)wPayload.c_str(), (DWORD)wPayload.length(),
        (DWORD)wPayload.length(), 0);

    if (!bResult) {
        std::string err = "WinHttpSendRequest FALLO. Codigo: " + std::to_string(GetLastError());
        DebugBox(err);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    WinHttpReceiveResponse(hRequest, NULL);

    // Codigo HTTP
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode,
        &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

    // Leer respuesta
    DWORD dwSize = 0;
    char buffer[4096];

    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;

        DWORD bytesRead = 0;
        if (WinHttpReadData(hRequest, buffer,
            std::min((DWORD)dwSize, (DWORD)(sizeof(buffer) - 1)), &bytesRead)) {
            buffer[bytesRead] = 0;
            response += buffer;
        }
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Debug con MessageBox
    std::stringstream debugMsg;
    debugMsg << "HTTP Status: " << statusCode << "\n\n";
    debugMsg << "Response (primeros 800 chars):\n";
    debugMsg << response.substr(0, 800);
    DebugBox(debugMsg.str());

    return response;
}

std::string MercadoPagoClient::CreatePaymentPreference(
    const std::string& title,
    const std::string& description,
    const std::string& price,
    const std::string& currency,
    const std::string& externalReference)
{
    std::stringstream ss;
    ss << "{"
       << "\"items\":[{"
       << "\"title\":\"" << title << "\","
       << "\"description\":\"" << description << "\","
       << "\"quantity\":1,"
       << "\"currency_id\":\"" << currency << "\","
       << "\"unit_price\":" << price
       << "}],"
       << "\"back_urls\":{"
       << "\"success\":\"https://www.google.com\","
       << "\"failure\":\"https://www.google.com\","
       << "\"pending\":\"https://www.google.com\""
       << "},"
       << "\"auto_return\":\"approved\","
       << "\"external_reference\":\"" << externalReference << "\""
       << "}";

    std::string payload = ss.str();
    std::string url = config_.baseUrl + "/checkout/preferences";

    DebugBox("Enviando a: " + url + "\n\nPayload:\n" + payload.substr(0, 400));

    std::string response = HttpPost(url, payload);

    // Buscar init_point
    size_t initPos = response.find("\"init_point\":\"");
    if (initPos != std::string::npos) {
        size_t start = initPos + 14;
        size_t end = response.find("\"", start);
        return response.substr(start, end - start);
    }

    // Alternativa: sandbox_init_point
    size_t sandboxPos = response.find("\"sandbox_init_point\":\"");
    if (sandboxPos != std::string::npos) {
        size_t start = sandboxPos + 22;
        size_t end = response.find("\"", start);
        return response.substr(start, end - start);
    }

    return "";
}