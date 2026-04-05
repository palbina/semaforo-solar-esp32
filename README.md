# Semáforo Solar IoT con Medición UV - ESP32

Sistema de semáforo solar inteligente (solmáforo) con ESP32, medición de radiación UV según estándares OMS, alimentación por panel solar y comunicación GSM/LTE. Incluye sensores meteorológicos adicionales y soporte LoRaWAN.

## Características

- **Microcontrolador**: ESP32 Dev Module
- **Comunicación**: SIM7000G (GSM/LTE) + LoRaWAN (SX1276)
- **Display**: Matriz LED 4x8x8 (MAX7219)
- **Medición Solar**: INA219 (precisa de corriente/potencia)
- **Indicador UV**: 5 LEDs según estándares OMS
- **Sensores**: Lluvia, Viento, UV, Temperatura, Humedad, GPS
- **Web**: Dashboard embebido con recomendaciones UV

## Solmáforo - Indicador UV (OMS)

| Color LED | Nivel | Índice UV | Recomendación |
|:---:|:---:|:---:|:---|
| 🟢 Verde | Bajo | 0 - 2 | Riesgo mínimo |
| 🟡 Amarillo | Moderado | 3 - 5 | Usar protector solar |
| 🟠 Naranja | Alto | 6 - 7 | Sombrero, gafas, protector |
| 🔴 Rojo | Muy Alto | 8 - 10 | Evitar sol 11:00-16:00 |
| 🟣 Violeta | Extremo | 11+ | Mantente en la sombra |

## Hardware

| Componente | Descripción |
|------------|-------------|
| ESP32 | Microcontrolador principal |
| SIM7000G | Módulo GSM/LTE + GPS |
| SX1276/RFM95 | Módulo LoRaWAN |
| MAX7219 | Display LED Matrix 4x8x8 |
| INA219 | Sensor corriente/potencia preciso |
| DHT11 | Sensor temperatura/humedad |
| UVM-30A | Sensor índice UV |
| FC-37 | Sensor lluvia |
| Anemómetro | Sensor velocidad viento |

## Sensores Meteorológicos

- **Lluvia**: FC-37 (detección digital)
- **Viento**: Anemómetro con pulsos (m/s)
- **UV**: UVM-30A (índice 0-11+)

## Niveles de Batería

| Nivel | Voltaje | Acción |
|-------|---------|--------|
| NORMAL | >= 12.0V | Operación normal |
| ADVERTENCIA | < 12.0V | Modo ahorro |
| BAJO | < 11.5V | Modo mínimo |
| CRÍTICO | < 11.0V | Deep sleep |

## MQTT Topics

```
semaforo/control    → Comandos (ROJO, VERDE, AMARILLO, PARPADEO)
semaforo/status     → Estado completo + UV + GPS
semaforo/solar      → Datos solares (INA219)
semaforo/gps        → Ubicación GPS
semaforo/alertas    → Alertas batería, temperatura
semaforo/lora       → Datos LoRaWAN
```

## Web Dashboard

Acceso vía IP del dispositivo:
- `/` - Dashboard con estado, UV y recomendaciones OMS
- `/api` - JSON con todos los datos incluyendo nivelUV
- `/control?cmd=X` - Control semáforo

## Pines

| Pin | Función |
|-----|---------|
| 4 | Power Key SIM7000G |
| 26, 27 | RX/TX Modem |
| 5 | CS Display |
| 12 | LED Rojo semáforo / UV |
| 13 | LED Amarillo / UV |
| 14 | LED Verde / UV |
| 15 | CS SD Card |
| 18, 23, 25 | LoRa (CS, RST, DIO0) |
| 21 | Sensor Lluvia |
| 22 | Sensor Viento (interruptor) |
| 39 | Sensor UV analógico |
| 34, 35 | Voltaje Batería, Panel |
| 33 | DHT11 |
| 36 | NTC Batería |

## Estructura

```
semaforo-solar-esp32/
├── platformio.ini
├── CONTEXT.md
├── src/
│   ├── main.cpp           # Programa principal
│   ├── sensores.cpp       # INA219, voltaje, NTC
│   ├── meteorologia.cpp   # Lluvia, viento, UV
│   ├── comunicaciones.cpp # GSM, WiFi, LoRa, MQTT
│   └── energia.cpp        # Batería, deep sleep, estadísticas
└── include/
    ├── config.h           # Pines y constantes
    ├── sensores.h
    ├── meteorologia.h
    ├── comunicaciones.h
    └── energia.h
```

## Instalación

```bash
# Clonar
git clone https://github.com/palbina/semaforo-solar-esp32.git
cd semaforo-solar-esp32

# Compilar
pio run

# Subir
pio run --target upload

# Monitor
pio device monitor
```

## OTA

- Host: IP del dispositivo
- Puerto: 3232
- Password: semaforo123

## Configuración LoRaWAN

- Frecuencia: 915 MHz (Chile)
- Spreading Factor: 7
- TTN compatible

## Estadísticas de Energía

El sistema registra en NVS:
- Wh consumidos
- Wh generados
- Ciclos de deep sleep

## Licencia

MIT License

## Autor

Pablo Albina (@palbina)