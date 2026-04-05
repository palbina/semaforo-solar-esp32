# Proyecto Semáforo Solar IoT - Contexto del Proyecto

## Resumen
Sistema de semáforo solar inteligente (solmáforo) con ESP32 conectado a red celular (SIM7000G), alimentado por panel solar y batería. Mide el índice de radiación UV según estándares OMS y muestra alertas visuales de 5 niveles. Controla LEDs de semáforo, display LED matrix, sensores meteorológicos, y se comunica vía MQTT, LoRaWAN y WiFi.

## Hardware
- **Microcontrolador**: ESP32 Dev Module
- **Módulo GSM/LTE**: SIM7000G
- **Módulo LoRa**: SX1276/RFM95 (868/915MHz)
- **Display**: Matriz LED 4x8x8 (MAX7219)
- **Sensor Corriente**: INA219 (preciso, 32V/2A)
- **Sensores**: 
  - Voltaje batería (divisor 100K/100K)
  - Voltaje panel solar (divisor 470K/100K)
  - Temperatura/Humedad ambiente (DHT11)
  - Temperatura batería (NTC 10K)
  - Sensor UV (UVM-30A)
  - Sensor Lluvia (FC-37)
  - Sensor Viento (anemómetro)
  - GPS (integrado en SIM7000G)
- **Almacenamiento**: SPIFFS (filesystem interno ESP32)
- **Actuadores**: 3 LEDs semáforo + 5 LEDs indicador UV

## Estados del Proyecto
- ✅ Proyecto base ESP32 + Arduino framework
- ✅ TinyGSM integrado para SIM7000G
- ✅ PubSubClient para MQTT
- ✅ Sensores de voltaje/corriente solar (INA219)
- ✅ Display LED Matrix MD_MAX72XX + MD_Parola
- ✅ Lógica de semáforo con tiempos configurables
- ✅ Control MQTT para cambiar estados
- ✅ Publicación de estado y datos solares por MQTT
- ✅ Sensor DHT11 (temperatura/humedad ambiente)
- ✅ Sensor NTC (temperatura batería)
- ✅ GPS integrado SIM7000G
- ✅ Web Server embebido (dashboard + API)
- ✅ OTA (Over-The-Air updates)
- ✅ Watchdog Timer (60s)
- ✅ Deep Sleep con protección de batería
- ✅ Preferences (config persistente en NVS)
- ✅ Reintentos exponenciales GPRS/MQTT
- ✅ Promedio ADC (5 muestras por lectura)
- ✅ **Sensor UV (indicador solmáforo OMS)**
- ✅ **Sensor Lluvia (FC-37)**
- ✅ **Sensor Viento (anemómetro)**
- ✅ **LoRaWAN (SX1276/RFM95)**
- ✅ **WiFi backup**
- ✅ **Estadísticas de energía (Wh)**
- ✅ **Display muestra UV, Solar, Ambiente, Semáforo**
- ✅ **Compilación verificada**

## Estructura de Archivos
```
SEMAFORO-SOLAR/
├── platformio.ini          # Configuración del proyecto
├── CONTEXT.md               # Documentación del proyecto
├── README.md                # README del proyecto
├── src/
│   ├── main.cpp           # Código principal
│   ├── sensores.cpp       # Gestión INA219, voltaje, NTC
│   ├── meteorologia.cpp   # Lluvia, viento, UV
│   ├── comunicaciones.cpp # GSM, WiFi, LoRa, MQTT
│   └── energia.cpp        # Gestión batería, deep sleep, estadísticas
├── include/
│   ├── config.h           # Definiciones de pines y constantes
│   ├── sensores.h         # Declaraciones sensores
│   ├── meteorologia.h     # Declaraciones meteorología
│   ├── comunicaciones.h   # Declaraciones comunicaciones
│   └── energia.h          # Declaraciones energía
├── lib/                   
├── include/               
└── test/                  
```

## Librerías en uso
| Librería | Versión | Propósito |
|----------|---------|-----------|
| TinyGSM | 0.12.0 | Comunicación SIM7000G |
| PubSubClient | 2.8.0 | Cliente MQTT |
| MD_MAX72xx | 3.5.1 | Control display LED |
| MD_Parola | 3.7.5 | Animaciones display |
| DHT sensor library | 1.4.7 | Sensor temperatura/humedad |
| Adafruit Unified Sensor | 1.1.15 | Sensor unified |
| Adafruit INA219 | 1.2.3 | Medición corriente/potencia precisa |
| LoRa (SandeepMistry) | 0.8.0 | Comunicación LoRaWAN |
| SPIFFS | 2.0.0 | Filesystem interno ESP32 |
| ArduinoOTA | 2.0.0 | Actualizaciones OTA |
| Preferences | 2.0.0 | Almacenamiento NVS |
| WebServer | 2.0.0 | Panel web embebido |
| SPI | 2.0.0 | Comunicación SPI |

## Configuración de Pines
| Pin | Función |
|-----|---------|
| 4 | Power Key SIM7000G |
| 26 | RX Modem SIM7000G |
| 27 | TX Modem SIM7000G |
| 5 | CS Display LED Matrix |
| 12 | LED Rojo semáforo / LED UV Rojo |
| 13 | LED Amarillo semáforo / LED UV Amarillo |
| 14 | LED Verde semáforo / LED UV Verde |
| 34 | Sensor Voltaje Batería |
| 35 | Sensor Voltaje Panel Solar |
| 33 | Sensor DHT11 |
| 36 | Sensor NTC Temperatura Batería |
| 2 | LoRa MOSI |
| 16 | LoRa MISO |
| 17 | LoRa SCK |
| 18 | LoRa CS |
| 23 | LoRa RST |
| 25 | LoRa DIO0 |
| 21 | Sensor Lluvia (digital) |
| 22 | Sensor Viento (interruptor) |
| 39 | Sensor UV (analógico) |
| 19 | LED UV Naranja |
| 26 | LED UV Violeta |

## Indicador UV (Solmáforo) - Estándar OMS
| Color LED | Nivel de Riesgo | Índice UV | Recomendación |
|:--- |:--- |:--- |:--- |
| **Verde** | Bajo | 0 - 2 | Riesgo mínimo. |
| **Amarillo** | Moderado | 3 - 5 | Aplicar protector solar si estarás mucho tiempo fuera. |
| **Naranja** | Alto | 6 - 7 | Usar sombrero, gafas y protector solar. |
| **Rojo** | Muy Alto | 8 - 10 | Evitar el sol entre las 11:00 y 16:00 hrs. |
| **Violeta** | Extremo | 11+ | Riesgo máximo. Mantente en la sombra. |

## Estados del Semáforo de Tráfico
```cpp
enum EstadoSemaforo {
  ESTADO_ROJO,           // configurable (default 30s)
  ESTADO_AMARILLO,       // configurable (default 5s)
  ESTADO_VERDE,          // configurable (default 25s)
  ESTADO_ROJO_PARPADEANDO  // Modo alerta
};
```

## Modos de Operación
```cpp
enum ModoOperacion {
  MODO_NORMAL,    // Todo activo
  MODO_AHORRO,    // Display apagado, publicación reducida
  MODO_MINIMO     // Solo LED rojo parpadeante, sin MQTT
};
```

## Niveles de Batería
| Nivel | Voltaje | Acción |
|-------|---------|--------|
| NORMAL | >= 12.0V | Operación normal |
| ADVERTENCIA | < 12.0V | Modo ahorro |
| BAJO | < 11.5V | Modo mínimo |
| CRÍTICO | < 11.0V | Deep sleep inmediato (10 min) |

## Variables Globales (definidas en main.cpp)
```cpp
float BATERIA_CRITICA = 11.0;
float BATERIA_BAJA = 11.5;
float BATERIA_ADVERTENCIA = 12.0;
float BATERIA_NORMAL = 12.5;

unsigned long TIEMPO_ROJO = 30000;
unsigned long TIEMPO_VERDE = 25000;
unsigned long TIEMPO_AMARILLO = 5000;
```

## Topics MQTT
| Topic | Dirección | Contenido |
|-------|-----------|-----------|
| semaforo/control | Entrada | "ROJO", "VERDE", "AMARILLO", "PARPADEO" |
| semaforo/status | Salida | JSON completo con estado, señal, gps, temp, UV |
| semaforo/solar | Salida | JSON con datos solares + temp batería |
| semaforo/gps | Salida | JSON con latitud, longitud |
| semaforo/alertas | Salida | JSON con alertas (batería, temp, OTA) |
| semaforo/lora | Salida | Datos enviados por LoRa |

## Payload MQTT Status
```json
{
  "estado": "VERDE",
  "senal": 25,
  "uptime": 3600,
  "bateria": 12.5,
  "panel": 18.2,
  "corriente": 0.45,
  "potencia": 5.6,
  "porcentaje": 80.0,
  "tempBat": 28.5,
  "temperatura": 25.0,
  "humedad": 65,
  "lluvia": 0,
  "viento": 2.5,
  "uv": 7.5,
  "lat": -34.603723,
  "lon": -58.381594
}
```

## Configuración Solar
- **Divisor Batería**: R1=100K, R2=100K (factor 2.0)
- **Divisor Panel**: R1=470K, R2=100K (factor 5.7)
- **INA219**: Dirección 0x40, calibración 32V/2A
- **Rango Batería**: 10.5V (0%) a 13.0V (100%)

## Configuración NTC Batería
- **Beta**: 3950
- **R serie**: 10KΩ
- **R referencia**: 10KΩ @ 25°C
- **Rango seguro**: 0°C a 45°C

## Configuración LoRaWAN (TTN)
- **Frecuencia**: 915000000 Hz (Chile)
- **Spreading Factor**: 7
- **Bandwidth**: 125000 Hz
- **Coding Rate**: 4/5
- **Sync Word**: 0x34
- **TX Power**: 20 dBm

## APN Configurados (Claro Argentina)
1. internet.claro.com.ar
2. wap.claro.com.ar
3. igprs.claro

## Funciones Principales
### Sensores
- `leerDatosSolar()` - Lee voltaje batería, panel, corriente (INA219)
- `leerDatosAmbiente()` - Lee DHT11
- `leerNTCTemperatura()` - Lee temperatura batería (NTC)
- `obtenerGPS()` - Obtiene coordenadas GPS
- `leerINA219()` - Lee datos precisos de corriente/potencia

### Meteorología
- `leerSensorLluvia()` - Detecta presencia de lluvia (FC-37)
- `leerSensorViento()` - Calcula velocidad del viento (anemómetro)
- `leerSensorUV()` - Lee índice UV (UVM-30A)
- `evaluarNivelUV()` - Determina nivel de riesgo OMS

### Gestión de Energía
- `evaluarNivelBateria()` - Evalúa estado de batería
- `evaluarTemperaturaBateria()` - Evalúa temp batería (NTC)
- `gestionarBateria()` - Gestiona modos según niveles
- `entrarDeepSleep()` - Entra en deep sleep
- `actualizarEstadisticas()` - Calcula Wh consumidos/generados

### Comunicación
- `iniciarWiFi()` - Inicia conexión WiFi
- `iniciarLoRa()` - Inicia módulo LoRa
- `iniciarModem()` - Inicia SIM7000G
- `conectarGPRS()` - Conecta a internet (backoff exponencial)
- `conectarMQTT()` - Conecta al broker
- `enviarLoRa()` - Envía datos por LoRa

### Display y Control
- `actualizarSemaforo()` - Controla LEDs y tiempos
- `actualizarIndicadorUV()` - Controla LEDs indicadores UV
- `mostrarEstadoSemaforo()` - Muestra estado en display
- `mostrarInfoSolar()` - Muestra datos solares en display
- `mostrarInfoAmbiente()` - Muestra temp/humedad en display
- `mostrarInfoUV()` - Muestra índice UV en display

### Sistema
- `iniciarWatchdog()` - Inicia watchdog timer (60s)
- `alimentarWatchdog()` - Resetea watchdog
- `iniciarOTA()` - Inicia servicio OTA
- `iniciarWebServer()` - Inicia panel web

## Web Server Endpoints
| Endpoint | Método | Descripción |
|----------|--------|-------------|
| `/` | GET | Dashboard HTML con estado, UV, recomendaciones |
| `/api` | GET | JSON con todos los datos incluyendo nivelUV |
| `/control?cmd=X` | GET | Cambiar estado semáforo |
| `/reset` | GET | Guardar config y reiniciar |

## Tiempos del Loop Principal
- Lectura sensores: cada 5 segundos (30s en modo mínimo)
- Actualización indicador UV: cada 5 segundos
- Publicación MQTT: cada 30 segundos
- Actualización display: cada 3 segundos (4 ciclos)
- Debug serial: cada 10 segundos (30s en modo mínimo)
- Publicación GPS: cada 5 minutos

## Deep Sleep
| Condición | Duración |
|-----------|----------|
| Batería crítica | 600 segundos |
| Batería baja sin sol | 300 segundos |
| Temperatura extrema | 300 segundos |

## Uso de Memoria (compilación real)
- **RAM**: 10.6% (34,860 bytes de 327,680 bytes)
- **Flash**: 44.9% (589,157 bytes de 1,310,720 bytes)

## Compilación
```bash
pio run                    # Compilar
pio run --target upload    # Subir a ESP32
pio device monitor          # Monitor serial
```

## OTA
- **Host**: Configurado automáticamente
- **Puerto**: 3232
- **Password**: semaforo123
- **Path actualización**: /update

## Configuración Persistente (NVS)
- `tiempoRojo` - Duración estado rojo (ms)
- `tiempoVerde` - Duración estado verde (ms)
- `tiempoAmarillo` - Duración estado amarillo (ms)
- `batCritica` - Umbral batería crítica (V)
- `batBaja` - Umbral batería baja (V)
- `batAdvertencia` - Umbral batería advertencia (V)
- `batNormal` - Umbral batería normal (V)

## Configuración Energía Solar (NVS)
- `whCons` - Wh consumidos acumulados
- `whGen` - Wh generados acumulados
- `dsCount` - Ciclos de deep sleep

## RTC Memory (persiste en deep sleep)
- `wakeUpCount` - Contador de despertares

## Errores Corregidos en Compilación
1. INA219: `setCalibration()` → `setCalibration_32V_2A()`
2. Multiple definitions: Variables definidas como `extern` en config.h e inicializadas en main.cpp
3. Pin names: Agregado alias `PIN_SENSOR_Viento` en config.h

## Ubicación de Uso
- **Ciudad**: Renca, Santiago, Chile
- **Características**: Zona con alta radiación UV, clima mediterráneo

## Repositorio
- **URL**: https://github.com/palbina/semaforo-solar-esp32
- **Último commit**: Fix compilation errors: INA219, multiple definitions, pin names