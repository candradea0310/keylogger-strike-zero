# Keylogger Strike Zero

Trabajo Parcial del curso **1CCB0001 - Hacking Ético** de la Universidad Peruana de Ciencias Aplicadas (UPC).

## Descripción

Proyecto académico que implementa una aplicación modular en C++ para el estudio de técnicas de seguridad ofensiva y defensiva en un entorno virtualizado controlado.

## Integrantes

- Andrade Alfaro, Camila Luciana (u202510132)
- Luna Fernandez, Leonardo Sebastian (u202519765)

## Docente

Custodio Chavarria, Gianpaul Jesus

## Semestre

2026-10

## Arquitectura del proyecto

El proyecto está compuesto por los siguientes módulos:

### Módulos principales
- `main.cpp` — Coordinador general de la aplicación.
- `keylogger.cpp` — Registro de eventos de entrada del teclado.
- `discord.cpp` — Comunicación HTTPS hacia canales externos.

### Módulos de captura
- `screenshots.cpp` — Registro periódico de pantalla (GDI+).
- `clipboard.cpp` — Registro del portapapeles.
- `browser.cpp` — Extracción de historial del navegador.
- `windowmonitor.cpp` — Monitoreo de ventanas activas.
- `credentials.cpp` — Registro de credenciales de prueba.

### Módulos de información del sistema
- `systeminfo.cpp` — Datos del sistema objetivo.
- `contadores.h` — Contadores globales.

### Módulos de simulación
- `ransomware_terminal.cpp` — Simulación de impacto (sin cifrado real).
- `graffiti.cpp` — Cambio demostrativo de fondo de pantalla.
- `blockkeys.cpp` — Bloqueo temporal de teclas.
- `mercadopago.cpp` — Simulación de interfaz de pago.

### Módulos auxiliares
- `config.h` — Configuración de canales de comunicación.
- `obfuscate.h` — Ofuscación de cadenas.

## Requisitos

- **Compilador:** MinGW-w64
- **Librerías:** WinHTTP, GDI+, Shell32, Ole32, Iphlpapi, Psapi
- **Sistema objetivo:** Windows 11

## Compilación

Desde Kali Linux (o cualquier sistema con MinGW-w64):

```bash
x86_64-w64-mingw32-g++ main.cpp discord.cpp keylogger.cpp screenshots.cpp \
    ransomware_terminal.cpp systeminfo.cpp windowmonitor.cpp gui.cpp \
    credentials.cpp mercadopago.cpp graffiti.cpp blockkeys.cpp \
    clipboard.cpp browser.cpp cripto.cpp \
    -o Mejora_Arma.exe \
    -lwinhttp -lcomctl32 -luser32 -ladvapi32 -lgdiplus -lgdi32 \
    -lshlwapi -lshell32 -lole32 -liphlpapi -lpsapi \
    -static -static-libgcc -static-libstdc++ \
    -mwindows -std=c++17 -Wno-write-strings
