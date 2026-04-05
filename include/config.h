#ifndef CONFIG_H
#define CONFIG_H

#define TINY_GSM_MODEM_SIM7000

#include <Arduino.h>

// ============= MODEM GSM (SIM7000G) =============
#define PIN_MODEM_TX 27
#define PIN_MODEM_RX 26
#define PIN_PWRKEY 4

// ============= LEDs SEMÁFORO =============
#define PIN_LED_RED 12
#define PIN_LED_YELLOW 13
#define PIN_LED_GREEN 14

// ============= LEDs INDICADOR UV (Solmáforo) =============
#define PIN_LED_UV_VERDE 14    // UV Bajo (0-2)
#define PIN_LED_UV_AMARILLO 13 // UV Moderado (3-5)
#define PIN_LED_UV_NARANJA 19  // UV Alto (6-7)
#define PIN_LED_UV_ROJO 12     // UV Muy Alto (8-10)
#define PIN_LED_UV_VIOLETA 26  // UV Extremo (11+)
#define LED_OFF HIGH
#define LED_ON LOW

// ============= SENSORES ANALÓGICOS ORIGINALES =============
#define PIN_ADC_BATERIA 34
#define PIN_ADC_PANEL 35
#define PIN_ADC_CURRENT 32
#define PIN_NTC_BATERIA 36

// ============= DISPLAY LED MATRIX =============
#define PIN_CS_DISPLAY 5
#define MAX_DEVICES 4

// ============= DHT11 =============
#define PIN_DHT 33
#define DHT_TYPE DHT11

// ============= SD CARD =============
#define PIN_CS_SD 15

// ============= LoRa (SX1276/RFM95) =============
#define PIN_LORA_MOSI 2
#define PIN_LORA_MISO 16
#define PIN_LORA_SCK 17
#define PIN_LORA_CS 18
#define PIN_LORA_RST 23
#define PIN_LORA_DIO0 25

// ============= SENSORES METEOROLÓGICOS =============
#define PIN_SENSOR_LLUVIA 21
#define PIN_SENSOR_Viento 22
#define PIN_SENSOR_Viento 22
#define PIN_SENSOR_UV 39

// ============= MQTT =============
#define MQTT_SERVER "broker.mqttdashboard.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "semaforo_solar_esp32"
#define MQTT_TOPIC_CONTROL "semaforo/control"
#define MQTT_TOPIC_STATUS "semaforo/status"
#define MQTT_TOPIC_SOLAR "semaforo/solar"
#define MQTT_TOPIC_GPS "semaforo/gps"
#define MQTT_TOPIC_ALERTAS "semaforo/alertas"
#define MQTT_TOPIC_LORA "semaforo/lora"

// ============= WiFi =============
#define WIFI_SSID "SemaforoWiFi"
#define WIFI_PASSWORD "semaforo123"
#define WIFI_TIMEOUT_MS 30000

// ============= LoRaWAN (TTN) =============
#define LORA_FREQUENCY 915000000
#define LORA_SPREADING_FACTOR 7
#define LORA_BANDWIDTH 125000
#define LORA_CODING_RATE 5
#define LORA_SYNC_WORD 0x34

// ============= ADC =============
#define ADC_SAMPLES 5

// ============= VOLTAJE BATERÍA (Original) =============
const float VOLTAJE_REFERENCIA = 3.3;
const int ADC_RESOLUTION = 4095;
const float DIVISOR_BATERIA_R1 = 100000.0;
const float DIVISOR_BATERIA_R2 = 100000.0;
const float FACTOR_BATERIA = (DIVISOR_BATERIA_R1 + DIVISOR_BATERIA_R2) / DIVISOR_BATERIA_R2;

// ============= VOLTAJE PANEL (Original) =============
const float DIVISOR_PANEL_R1 = 470000.0;
const float DIVISOR_PANEL_R2 = 100000.0;
const float FACTOR_PANEL = (DIVISOR_PANEL_R1 + DIVISOR_PANEL_R2) / DIVISOR_PANEL_R2;

// ============= NTC BATERÍA =============
const float NTC_BETA = 3950.0;
const float NTC_SERIE_R = 10000.0;
const float NTC_REF_TEMP = 25.0;
const float NTC_REF_RES = 10000.0;

// ============= NIVELES BATERÍA =============
extern float BATERIA_CRITICA;
extern float BATERIA_BAJA;
extern float BATERIA_ADVERTENCIA;
extern float BATERIA_NORMAL;

// ============= TEMPERATURA BATERÍA =============
#define TEMP_BATERIA_MIN 0.0
#define TEMP_BATERIA_MAX 45.0

// ============= DEEP SLEEP =============
#define DEEPSLEEP_SEGUNDOS 300
#define DEEPSLEEP_CRITICO_SEGUNDOS 600

// ============= TIEMPOS SEMÁFORO =============
extern unsigned long TIEMPO_ROJO;
extern unsigned long TIEMPO_VERDE;
extern unsigned long TIEMPO_AMARILLO;

// ============= INA219 =============
#define INA219_ADDRESS 0x40
const float INA219_SHUNT_OHMS = 0.1;
const float INA219_MAX_AMPS = 0.4;

// ============= SENSOR UV =============
const float UV_MAX_VOLTAGE = 3.3;
const float UV_MAX_INDEX = 11.0;

// ============= ANEMÓMETRO =============
#define VIENTO_PULSOS_REVOLUCION 1
#define DIAMETRO_ANEMOMETRO_CM 9.0

// ============= ENUMERACIONES =============
enum EstadoSemaforo {
    ESTADO_ROJO,
    ESTADO_AMARILLO,
    ESTADO_VERDE,
    ESTADO_ROJO_PARPADEANDO
};

enum NivelBateria {
    NIVEL_NORMAL,
    NIVEL_ADVERTENCIA,
    NIVEL_BAJO,
    NIVEL_CRITICO
};

enum ModoOperacion {
    MODO_NORMAL,
    MODO_AHORRO,
    MODO_MINIMO
};

enum TipoConexion {
    CONEXION_GSM,
    CONEXION_WIFI,
    CONEXION_LORA
};

enum EstadoLluvia {
    LLUVIA_SECO,
    LLUVIA_LLOVIZNA,
    LLUVIA_LLUVIA
};

enum NivelUV {
    UV_BAJO,
    UV_MODERADO,
    UV_ALTO,
    UV_MUY_ALTO,
    UV_EXTREMO
};

#define UV_BAJO_MAX 2
#define UV_MODERADO_MAX 5
#define UV_ALTO_MAX 7
#define UV_MUY_ALTO_MAX 10

#endif // CONFIG_H