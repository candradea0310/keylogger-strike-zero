// ============================================================
//  obfuscate.h — Ofuscación de strings en tiempo de compilación
//  Basado en el concepto del ofuscador.py (multi-capa)
// ============================================================
#pragma once
#include <windows.h>
#include <string>

namespace Obf {

constexpr char KEY1 = 0x5A;
constexpr char KEY2 = 0xA7;
constexpr char KEY3 = 0x3C;

template <size_t N>
struct ObfuscatedString {
    char data[N];
    constexpr ObfuscatedString(const char (&str)[N]) : data{} {
        for (size_t i = 0; i < N; i++) {
            char c = str[i];
            c ^= KEY1;
            c = (char)((c + 13) & 0xFF);
            c ^= KEY2;
            c = (char)((c - 7) & 0xFF);
            c ^= KEY3;
            data[i] = c;
        }
    }

    std::string Get() const {
        std::string result;
        for (size_t i = 0; i < N; i++) {
            char c = data[i];
            c ^= KEY3;
            c = (char)((c + 7) & 0xFF);
            c ^= KEY2;
            c = (char)((c - 13) & 0xFF);
            c ^= KEY1;
            if (c == 0) break;
            result += c;
        }
        return result;
    }
};

template <size_t N>
constexpr auto Make(const char (&str)[N]) {
    return ObfuscatedString<N>(str);
}

}

#define OBF(str) Obf::Make(str).Get()