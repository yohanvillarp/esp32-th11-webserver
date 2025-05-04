# 🌤️ Estación Meteorológica con ESP32 y DHT11

Este proyecto crea una estación meteorológica básica utilizando un ESP32 y un sensor DHT11. Los datos se muestran en un servidor web local accesible desde cualquier navegador dentro de la red WiFi.

## 📌 Funcionalidades

- Lectura de:
  - Temperatura (°C, °F, Kelvin, Rankine)
  - Humedad relativa (%)
- Cálculos adicionales:
  - Punto de rocío
  - Humedad absoluta
  - Sensación térmica
  - Bulbo húmedo
  - Presión de vapor
  - Presión de saturación
- Servidor web embebido con recarga automática cada 5 segundos

## 🔧 Requisitos

- ESP32
- Sensor DHT11
- Arduino IDE
- Librerías:
  - `DHT.h`
  - `WiFi.h`

## ⚙️ Configuración

1. Cambia las credenciales WiFi en el código:

```cpp
const char* ssid = "TuSSID";
const char* password = "TuContraseña";
