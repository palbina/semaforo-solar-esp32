#include <Arduino.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <DHT.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <Wire.h>
#include <SPIFFS.h>

#include "config.h"
#include "sensores.h"
#include "meteorologia.h"
#include "comunicaciones.h"
#include "energia.h"

float BATERIA_CRITICA = 11.0;
float BATERIA_BAJA = 11.5;
float BATERIA_ADVERTENCIA = 12.0;
float BATERIA_NORMAL = 12.5;

unsigned long TIEMPO_ROJO = 30000;
unsigned long TIEMPO_VERDE = 25000;
unsigned long TIEMPO_AMARILLO = 5000;

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);
PubSubClient mqttClient(gsmClient);

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, PIN_CS_DISPLAY, MAX_DEVICES);
MD_Parola display = MD_Parola(MD_MAX72XX::FC16_HW, PIN_CS_DISPLAY, MAX_DEVICES);
DHT dht(PIN_DHT, DHT_TYPE);
WebServer server(80);

Sensores sensores;
Meteorologia meteorologia;
Comunicaciones comunicaciones;
Energia energia;

RTC_DATA_ATTR uint32_t wakeUpCount = 0;

NivelUV evaluarNivelUV(float indiceUV);

EstadoSemaforo estadoActual = ESTADO_ROJO;
unsigned long ultimoCambio = 0;

bool gsmConectado = false;
bool mqttConectado = false;

DatosSolar datosSolar = {0, 0, 0, 0, 0, 0, 0};
DatosAmbiente datosAmbiente = {0, 0};
DatosMeteorologicos datosMeteo = {LLUVIA_SECO, 0, 0, false, false};
DatosGPS datosGPS = {0, 0, false};

float temperaturaBateria = 25.0;

int obtenerRetryExponencial(int intento, int baseMs = 1000, int maxMs = 60000) {
    int delayTime = baseMs * pow(2, intento);
    return min(delayTime, maxMs);
}

void guardarConfig() {
    Preferences pref;
    pref.begin("semaforo", false);
    pref.putFloat("tiempoRojo", TIEMPO_ROJO);
    pref.putFloat("tiempoVerde", TIEMPO_VERDE);
    pref.putFloat("tiempoAmarillo", TIEMPO_AMARILLO);
    pref.putFloat("batCritica", BATERIA_CRITICA);
    pref.putFloat("batBaja", BATERIA_BAJA);
    pref.putFloat("batAdvertencia", BATERIA_ADVERTENCIA);
    pref.putFloat("batNormal", BATERIA_NORMAL);
    pref.end();
    Serial.println("Configuracion guardada en NVS");
}

void cargarConfig() {
    Preferences pref;
    pref.begin("semaforo", true);
    TIEMPO_ROJO = pref.getFloat("tiempoRojo", 30000);
    TIEMPO_VERDE = pref.getFloat("tiempoVerde", 25000);
    TIEMPO_AMARILLO = pref.getFloat("tiempoAmarillo", 5000);
    BATERIA_CRITICA = pref.getFloat("batCritica", 11.0);
    BATERIA_BAJA = pref.getFloat("batBaja", 11.5);
    BATERIA_ADVERTENCIA = pref.getFloat("batAdvertencia", 12.0);
    BATERIA_NORMAL = pref.getFloat("batNormal", 12.5);
    pref.end();
    Serial.println("Configuracion cargada desde NVS");
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Semafoto Solar IoT</title>";
    html += "<style>body{font-family:Arial;padding:20px;background:#f0f0f0}";
    html += "h1{color:#333}.card{background:#fff;padding:15px;border-radius:8px;margin:10px 0;box-shadow:0 2px 5px rgba(0,0,0,0.1)}";
    html += ".status{font-size:24px;font-weight:bold}.ok{color:#4CAF50}.warn{color:#ff9800}.crit{color:#f44336}";
    html += "table{width:100%;border-collapse:collapse}td{padding:8px;border-bottom:1px solid #ddd}";
    html += ".btn{background:#4CAF50;color:#fff;padding:10px 20px;border:none;border-radius:5px;cursor:pointer;margin:5px}";
    html += ".btn:hover{background:#45a049}</style></head><body>";
    html += "<h1>Semafoto Solar IoT</h1>";
    
    ModoOperacion modo = energia.getModoOperacion();
    NivelBateria nivel = energia.getNivelBateria();
    
    const char* modoStr = (modo == MODO_NORMAL) ? "NORMAL" : (modo == MODO_AHORRO) ? "AHORRO" : "MINIMO";
    const char* nivelStr = (nivel == NIVEL_NORMAL) ? "OK" : (nivel == NIVEL_ADVERTENCIA) ? "ADVERTENCIA" : (nivel == NIVEL_BAJO) ? "BAJA" : "CRITICO";
    const char* nivelClass = (nivel == NIVEL_NORMAL) ? "ok" : (nivel == NIVEL_ADVERTENCIA) ? "warn" : "crit";
    const char* estadoStr = (estadoActual == ESTADO_ROJO) ? "ROJO" : (estadoActual == ESTADO_VERDE) ? "VERDE" : (estadoActual == ESTADO_AMARILLO) ? "AMARILLO" : "PARPADEANDO";
    const char* lluviaStr = (datosMeteo.lluvia == LLUVIA_SECO) ? "SECO" : (datosMeteo.lluvia == LLUVIA_LLOVIZNA) ? "LLOVIZNA" : "LLUVIA";
    NivelUV nivelUV = evaluarNivelUV(datosMeteo.indiceUV);
    const char* nivelUVStr = (nivelUV == UV_BAJO) ? "BAJO" : 
                             (nivelUV == UV_MODERADO) ? "MODERADO" :
                             (nivelUV == UV_ALTO) ? "ALTO" :
                             (nivelUV == UV_MUY_ALTO) ? "MUY ALTO" : "EXTREMO";
    const char* uvClass = (nivelUV == UV_BAJO) ? "ok" : 
                          (nivelUV == UV_MODERADO) ? "warn" :
                          (nivelUV == UV_ALTO) ? "crit" : "crit";
    
    html += "<div class='card'><h2>Estado</h2>";
    html += "<p class='status'>Modo: " + String(modoStr) + "</p>";
    html += "<p class='status'>Semaforo: " + String(estadoStr) + "</p>";
    html += "<p class='status " + String(nivelClass) + "'>Bateria: " + String(nivelStr) + "</p>";
    html += "<p class='status " + String(uvClass) + "'>UV: " + String(nivelUVStr) + " (" + String(datosMeteo.indiceUV, 1) + ")</p>";
    html += "<p>Lluvia: " + String(lluviaStr) + " | Viento: " + String(datosMeteo.velocidadViento, 1) + " m/s</p>";
    html += "<p>Wake Count: " + String(wakeUpCount) + "</p></div>";
    
    html += "<div class='card'><h2>Datos Solares (INA219)</h2>";
    html += "<table><tr><td>Bateria</td><td>" + String(datosSolar.voltajeBateria, 2) + "V (" + String(datosSolar.porcentajeBateria, 1) + "%)</td></tr>";
    html += "<tr><td>Panel Solar</td><td>" + String(datosSolar.voltajePanel, 2) + "V</td></tr>";
    html += "<tr><td>Corriente</td><td>" + String(datosSolar.corriente, 3) + "A</td></tr>";
    html += "<tr><td>Potencia</td><td>" + String(datosSolar.potencia, 2) + "W</td></tr>";
    html += "<tr><td>Temp Bateria</td><td>" + String(temperaturaBateria, 1) + "C</td></tr>";
    html += "<tr><td>Temp Ambiente</td><td>" + String(datosAmbiente.temperatura, 1) + "C</td></tr>";
    html += "<tr><td>Humedad</td><td>" + String(datosAmbiente.humedad, 0) + "%</td></tr>";
    if (datosGPS.valido) {
        html += "<tr><td>GPS</td><td>" + String(datosGPS.latitud, 6) + ", " + String(datosGPS.longitud, 6) + "</td></tr>";
    }
    html += "</table></div>";
    
    const char* recomendacionUV;
    switch (nivelUV) {
        case UV_BAJO: recomendacionUV = "Riesgo minimo"; break;
        case UV_MODERADO: recomendacionUV = "Usar protector si estas mucho tiempo fuera"; break;
        case UV_ALTO: recomendacionUV = "Usar sombrero, gafas y protector solar"; break;
        case UV_MUY_ALTO: recomendacionUV = "Evitar sol entre 11:00 y 16:00 hrs"; break;
        case UV_EXTREMO: recomendacionUV = "Riesgo maximo. Mantente en la sombra"; break;
    }
    html += "<div class='card'><h2>Recomendacion UV</h2>";
    html += "<p style='font-size:18px;color:#333'>" + String(recomendacionUV) + "</p></div>";
    
    html += "<div class='card'><h2>Control</h2>";
    html += "<a href='/control?cmd=ROJO'><button class='btn'>ROJO</button></a>";
    html += "<a href='/control?cmd=VERDE'><button class='btn'>VERDE</button></a>";
    html += "<a href='/control?cmd=AMARILLO'><button class='btn'>AMARILLO</button></a>";
    html += "<a href='/control?cmd=PARPADEO'><button class='btn'>PARPADEO</button></a>";
    html += "<a href='/reset'><button class='btn' style='background:#f44336'>RESET</button></a>";
    html += "</div></body></html>";
    
    server.send(200, "text/html", html);
}

void handleControl() {
    if (server.hasArg("cmd")) {
        String cmd = server.arg("cmd");
        if (cmd == "ROJO") {
            estadoActual = ESTADO_ROJO;
            ultimoCambio = millis();
        } else if (cmd == "VERDE") {
            estadoActual = ESTADO_VERDE;
            ultimoCambio = millis();
        } else if (cmd == "AMARILLO") {
            estadoActual = ESTADO_AMARILLO;
            ultimoCambio = millis();
        } else if (cmd == "PARPADEO") {
            estadoActual = ESTADO_ROJO_PARPADEANDO;
        }
        Serial.println("Control web: " + cmd);
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void handleReset() {
    guardarConfig();
    server.send(200, "text/plain", "Config guardada, reiniciando...");
    delay(1000);
    ESP.restart();
}

void handleApi() {
    EnergiaStats stats = energia.getEstadisticas();
    const char* lluviaStr = (datosMeteo.lluvia == LLUVIA_SECO) ? "SECO" : "LLUVIA";
    NivelUV nivelUV = evaluarNivelUV(datosMeteo.indiceUV);
    const char* nivelUVStr = (nivelUV == UV_BAJO) ? "BAJO" : 
                             (nivelUV == UV_MODERADO) ? "MODERADO" :
                             (nivelUV == UV_ALTO) ? "ALTO" :
                             (nivelUV == UV_MUY_ALTO) ? "MUY_ALTO" : "EXTREMO";
    
    String json = "{";
    json += "\"estado\":\"" + String((estadoActual == ESTADO_ROJO) ? "ROJO" : (estadoActual == ESTADO_VERDE) ? "VERDE" : (estadoActual == ESTADO_AMARILLO) ? "AMARILLO" : "PARPADEANDO") + "\",";
    json += "\"modo\":\"" + String((energia.getModoOperacion() == MODO_NORMAL) ? "NORMAL" : (energia.getModoOperacion() == MODO_AHORRO) ? "AHORRO" : "MINIMO") + "\",";
    json += "\"bateria\":" + String(datosSolar.voltajeBateria, 2) + ",";
    json += "\"porcentaje\":" + String(datosSolar.porcentajeBateria, 1) + ",";
    json += "\"panel\":" + String(datosSolar.voltajePanel, 2) + ",";
    json += "\"corriente\":" + String(datosSolar.corriente, 3) + ",";
    json += "\"potencia\":" + String(datosSolar.potencia, 2) + ",";
    json += "\"tempBat\":" + String(temperaturaBateria, 1) + ",";
    json += "\"tempAmbiente\":" + String(datosAmbiente.temperatura, 1) + ",";
    json += "\"humedad\":" + String(datosAmbiente.humedad, 0) + ",";
    json += "\"lluvia\":\"" + String(lluviaStr) + "\",";
    json += "\"viento\":" + String(datosMeteo.velocidadViento, 1) + ",";
    json += "\"uv\":" + String(datosMeteo.indiceUV, 1) + ",";
    json += "\"nivelUV\":\"" + String(nivelUVStr) + "\",";
    json += "\"whConsumidos\":" + String(stats.whConsumidos, 2) + ",";
    json += "\"whGenerados\":" + String(stats.whGenerados, 2) + ",";
    json += "\"wakeUpCount\":" + String(wakeUpCount);
    json += "}";
    server.send(200, "application/json", json);
}

void iniciarWebServer() {
    server.on("/", handleRoot);
    server.on("/control", handleControl);
    server.on("/reset", handleReset);
    server.on("/api", handleApi);
    server.begin();
    Serial.println("Web server iniciado");
}

void iniciarOTA() {
    ArduinoOTA.setHostname(MQTT_CLIENT_ID);
    ArduinoOTA.setPassword("semaforo123");
    
    ArduinoOTA.onStart([]() {
        Serial.println("OTA: Inicio actualizacion");
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA: Actualizacion completada");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA: %u%%\n", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error: %d\n", error);
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA habilitado");
}

void iniciarWatchdog() {
    esp_task_wdt_init(60, true);
    esp_task_wdt_add(NULL);
    Serial.println("Watchdog timer iniciado (60s)");
}

void alimentarWatchdog() {
    esp_task_wdt_reset();
}

void iniciarDisplay() {
    mx.begin();
    display.begin();
    display.setIntensity(7);
    display.setInvert(false);
    display.displayClear();
    display.displayScroll("SEMAFORO SOLAR", PA_CENTER, PA_SCROLL_LEFT, 50);
    delay(2000);
    display.displayClear();
    Serial.println("Display MAX7219 inicializado");
}

void mostrarEnDisplay(const char* texto) {
    if (display.displayAnimate()) {
        display.displayClear();
        display.displayScroll(texto, PA_CENTER, PA_SCROLL_LEFT, 50);
    }
}

void mostrarEstadoSemaforo() {
    switch (estadoActual) {
        case ESTADO_ROJO:
            mostrarEnDisplay("PARAR");
            break;
        case ESTADO_VERDE:
            mostrarEnDisplay("AVANZAR");
            break;
        case ESTADO_AMARILLO:
            mostrarEnDisplay("PRECAUCION");
            break;
        case ESTADO_ROJO_PARPADEANDO:
            mostrarEnDisplay("ALERTA");
            break;
    }
}

void mostrarInfoSolar() {
    char buffer[100];
    
    if (display.displayAnimate()) {
        display.displayClear();
        snprintf(buffer, sizeof(buffer), "Bat:%.1f%% %.1fV", 
            datosSolar.porcentajeBateria, datosSolar.voltajeBateria);
        display.displayScroll(buffer, PA_CENTER, PA_SCROLL_UP, 40);
    }
}

void mostrarInfoAmbiente() {
    char buffer[100];
    
    if (display.displayAnimate()) {
        display.displayClear();
        snprintf(buffer, sizeof(buffer), "T:%.1fC H:%.0f%%", 
            datosAmbiente.temperatura, datosAmbiente.humedad);
        display.displayScroll(buffer, PA_CENTER, PA_SCROLL_UP, 40);
    }
}

void actualizarSemaforo() {
    digitalWrite(PIN_LED_RED, LED_OFF);
    digitalWrite(PIN_LED_YELLOW, LED_OFF);
    digitalWrite(PIN_LED_GREEN, LED_OFF);
    
    unsigned long tiempoTranscurrido = millis() - ultimoCambio;
    
    switch (estadoActual) {
        case ESTADO_ROJO:
            digitalWrite(PIN_LED_RED, LED_ON);
            if (tiempoTranscurrido >= TIEMPO_ROJO) {
                estadoActual = ESTADO_VERDE;
                ultimoCambio = millis();
            }
            break;
            
        case ESTADO_VERDE:
            digitalWrite(PIN_LED_GREEN, LED_ON);
            if (tiempoTranscurrido >= TIEMPO_VERDE) {
                estadoActual = ESTADO_AMARILLO;
                ultimoCambio = millis();
            }
            break;
            
        case ESTADO_AMARILLO:
            digitalWrite(PIN_LED_YELLOW, LED_ON);
            if (tiempoTranscurrido >= TIEMPO_AMARILLO) {
                estadoActual = ESTADO_ROJO;
                ultimoCambio = millis();
            }
            break;
            
        case ESTADO_ROJO_PARPADEANDO:
            if ((millis() / 500) % 2 == 0) {
                digitalWrite(PIN_LED_RED, LED_ON);
            } else {
                digitalWrite(PIN_LED_RED, LED_OFF);
            }
            break;
    }
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje MQTT en topic: ");
    Serial.println(topic);
    
    char mensaje[length + 1];
    for (unsigned int i = 0; i < length; i++) {
        mensaje[i] = (char)payload[i];
    }
    mensaje[length] = '\0';
    
    Serial.println("Mensaje: " + String(mensaje));
    
    if (strcmp(topic, MQTT_TOPIC_CONTROL) == 0) {
        if (strcmp(mensaje, "ROJO") == 0) {
            estadoActual = ESTADO_ROJO;
            ultimoCambio = millis();
        } else if (strcmp(mensaje, "VERDE") == 0) {
            estadoActual = ESTADO_VERDE;
            ultimoCambio = millis();
        } else if (strcmp(mensaje, "AMARILLO") == 0) {
            estadoActual = ESTADO_AMARILLO;
            ultimoCambio = millis();
        } else if (strcmp(mensaje, "PARPADEO") == 0) {
            estadoActual = ESTADO_ROJO_PARPADEANDO;
        }
    }
}

bool iniciarSDCard() {
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: No se pudo inicializar SPIFFS");
        return false;
    }
    Serial.println("SPIFFS inicializado");
    
    File root = SPIFFS.open("/");
    if (!root) {
        Serial.println("ERROR: No se pudo abrir raiz SPIFFS");
        return false;
    }
    
    File file = root.openNextFile();
    if (!file) {
        Serial.println("Creando archivo de logs...");
        File logFile = SPIFFS.open("/log.txt", FILE_WRITE);
        if (logFile) {
            logFile.println("=== Log SPIFFS Inicializado ===");
            logFile.close();
            Serial.println("Archivo log.txt creado");
        }
    }
    root.close();
    
    Serial.println("SPIFFS listo para uso");
    return true;
}

void escribirLogSD(const char* mensaje) {
    File logFile = SPIFFS.open("/log.txt", FILE_APPEND);
    if (logFile) {
        logFile.print(millis());
        logFile.print(": ");
        logFile.println(mensaje);
        logFile.close();
    }
}

NivelUV evaluarNivelUV(float indiceUV) {
    if (indiceUV <= UV_BAJO_MAX) return UV_BAJO;
    if (indiceUV <= UV_MODERADO_MAX) return UV_MODERADO;
    if (indiceUV <= UV_ALTO_MAX) return UV_ALTO;
    if (indiceUV <= UV_MUY_ALTO_MAX) return UV_MUY_ALTO;
    return UV_EXTREMO;
}

void actualizarIndicadorUV(float indiceUV) {
    NivelUV nivel = evaluarNivelUV(indiceUV);
    
    digitalWrite(PIN_LED_UV_VERDE, LED_OFF);
    digitalWrite(PIN_LED_UV_AMARILLO, LED_OFF);
    digitalWrite(PIN_LED_UV_NARANJA, LED_OFF);
    digitalWrite(PIN_LED_UV_ROJO, LED_OFF);
    digitalWrite(PIN_LED_UV_VIOLETA, LED_OFF);
    
    const char* nivelStr;
    const char* recomendacion;
    
    switch (nivel) {
        case UV_BAJO:
            digitalWrite(PIN_LED_UV_VERDE, LED_ON);
            nivelStr = "BAJO";
            recomendacion = "Riesgo minimo";
            break;
        case UV_MODERADO:
            digitalWrite(PIN_LED_UV_AMARILLO, LED_ON);
            nivelStr = "MODERADO";
            recomendacion = "Usar protector si estas mucho tiempo fuera";
            break;
        case UV_ALTO:
            digitalWrite(PIN_LED_UV_NARANJA, LED_ON);
            nivelStr = "ALTO";
            recomendacion = "Usar sombrero, gafas y protector solar";
            break;
        case UV_MUY_ALTO:
            digitalWrite(PIN_LED_UV_ROJO, LED_ON);
            nivelStr = "MUY ALTO";
            recomendacion = "Evitar sol entre 11:00 y 16:00 hrs";
            break;
        case UV_EXTREMO:
            digitalWrite(PIN_LED_UV_VIOLETA, LED_ON);
            nivelStr = "EXTREMO";
            recomendacion = "Riesgo maximo. Mantente en la sombra";
            break;
    }
    
    Serial.printf("UV: %.1f - Nivel: %s - %s\n", indiceUV, nivelStr, recomendacion);
}

void mostrarInfoUV() {
    char buffer[100];
    
    if (display.displayAnimate()) {
        display.displayClear();
        snprintf(buffer, sizeof(buffer), "UV:%.1f", datosMeteo.indiceUV);
        display.displayScroll(buffer, PA_CENTER, PA_SCROLL_UP, 40);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== SEMAFORO SOLAR ESP32 v4.0 ===");
    
    cargarConfig();
    
    wakeUpCount++;
    Serial.printf("Wake count: %u\n", wakeUpCount);
    
    esp_sleep_wakeup_cause_t causa = esp_sleep_get_wakeup_cause();
    if (causa == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Despertado por timer (deep sleep)");
    } else if (causa == ESP_SLEEP_WAKEUP_UNDEFINED) {
        Serial.println("Primer arranque");
    }
    
    energia.init();
    energia.cargarConfig();
    
    iniciarWatchdog();
    
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    
    // Inicializar LEDs del indicador UV
    pinMode(PIN_LED_UV_VERDE, OUTPUT);
    pinMode(PIN_LED_UV_AMARILLO, OUTPUT);
    pinMode(PIN_LED_UV_NARANJA, OUTPUT);
    pinMode(PIN_LED_UV_ROJO, OUTPUT);
    pinMode(PIN_LED_UV_VIOLETA, OUTPUT);
    
    digitalWrite(PIN_LED_RED, LED_OFF);
    digitalWrite(PIN_LED_YELLOW, LED_OFF);
    digitalWrite(PIN_LED_GREEN, LED_OFF);
    digitalWrite(PIN_LED_UV_VERDE, LED_OFF);
    digitalWrite(PIN_LED_UV_AMARILLO, LED_OFF);
    digitalWrite(PIN_LED_UV_NARANJA, LED_OFF);
    digitalWrite(PIN_LED_UV_ROJO, LED_OFF);
    digitalWrite(PIN_LED_UV_VIOLETA, LED_OFF);
    
    iniciarSDCard();
    
    sensores.init();
    datosSolar = sensores.leerDatosSolar();
    temperaturaBateria = sensores.leerNTCTemperatura();
    
    Serial.printf("Bateria: %.2fV (%.1f%%) | Corriente: %.3fA | Temp: %.1fC\n", 
        datosSolar.voltajeBateria, datosSolar.porcentajeBateria, datosSolar.corriente, temperaturaBateria);
    Serial.printf("Panel solar: %.2fV | Potencia: %.2fW\n", datosSolar.voltajePanel, datosSolar.potencia);
    
    NivelBateria nivel = energia.evaluarNivelBateria(datosSolar.voltajeBateria);
    
    if (nivel == NIVEL_CRITICO || energia.evaluarTemperaturaBateria(temperaturaBateria)) {
        Serial.println("BATERIA/TEMP CRITICA - Deep sleep inmediato");
        delay(1000);
        energia.entrarDeepSleep(DEEPSLEEP_CRITICO_SEGUNDOS);
    }
    
    if (nivel == NIVEL_BAJO && datosSolar.voltajePanel < 5.0) {
        Serial.println("Bateria baja + sin sol - Deep sleep");
        delay(1000);
        energia.entrarDeepSleep(DEEPSLEEP_SEGUNDOS);
    }
    
    if (nivel >= NIVEL_BAJO) {
        energia.gestionarBateria(datosSolar.voltajeBateria, temperaturaBateria, datosSolar.voltajePanel);
    }
    
    dht.begin();
    
    meteorologia.init();
    datosMeteo = meteorologia.leerDatos();
    
    comunicaciones.init();
    
    iniciarDisplay();
    
    comunicaciones.iniciarLoRa();
    
    if (comunicaciones.iniciarModem()) {
        if (comunicaciones.conectarGPRS()) {
            if (comunicaciones.conectarMQTT(mqttClient, gsmClient)) {
                mqttConectado = true;
            }
        }
    }
    
    if (energia.getModoOperacion() != MODO_MINIMO) {
        iniciarWebServer();
        iniciarOTA();
        comunicaciones.obtenerGPS(datosGPS);
    }
    
    ultimoCambio = millis();
    Serial.println("Sistema iniciado");
}

void loop() {
    alimentarWatchdog();
    
    static unsigned long lastSolarRead = 0;
    unsigned long intervalRead = (energia.getModoOperacion() == MODO_MINIMO) ? 30000 : 5000;
    
    if (millis() - lastSolarRead > intervalRead) {
        datosSolar = sensores.leerDatosSolar();
        datosAmbiente = sensores.leerDatosAmbiente();
        datosMeteo = meteorologia.leerDatos();
        lastSolarRead = millis();
        
        energia.gestionarBateria(datosSolar.voltajeBateria, temperaturaBateria, datosSolar.voltajePanel);
        
        energia.actualizarEstadisticas(datosSolar.corriente, intervalRead / 1000.0);
        
        if (energia.getModoOperacion() != MODO_MINIMO) {
            actualizarIndicadorUV(datosMeteo.indiceUV);
        }
    }
    
    if (energia.getModoOperacion() == MODO_NORMAL) {
        if (!mqttClient.connected()) {
            mqttConectado = false;
            int intento = 0;
            while (!comunicaciones.conectarMQTT(mqttClient, gsmClient) && intento < 3) {
                intento++;
                delay(obtenerRetryExponencial(intento, 2000, 60000));
            }
            if (mqttClient.connected()) mqttConectado = true;
        }
        
        if (mqttConectado) {
            mqttClient.loop();
            server.handleClient();
            ArduinoOTA.handle();
            
            static unsigned long lastPublish = 0;
            if (millis() - lastPublish > 30000) {
                char payload[400];
                snprintf(payload, sizeof(payload),
                    "{\"estado\":\"%s\",\"senal\":%d,\"uptime\":%lu,\"bateria\":%.2f,\"panel\":%.2f,\"corriente\":%.3f,\"potencia\":%.2f,\"porcentaje\":%.1f,\"tempBat\":%.1f,\"temperatura\":%.1f,\"humedad\":%.0f,\"lluvia\":%d,\"viento\":%.1f,\"uv\":%.1f,\"lat\":%.6f,\"lon\":%.6f}",
                    (estadoActual == ESTADO_ROJO) ? "ROJO" : (estadoActual == ESTADO_VERDE) ? "VERDE" : (estadoActual == ESTADO_AMARILLO) ? "AMARILLO" : "PARPADEANDO",
                    comunicaciones.obtenerSignalQuality(),
                    millis() / 1000,
                    datosSolar.voltajeBateria,
                    datosSolar.voltajePanel,
                    datosSolar.corriente,
                    datosSolar.potencia,
                    datosSolar.porcentajeBateria,
                    temperaturaBateria,
                    datosAmbiente.temperatura,
                    datosAmbiente.humedad,
                    datosMeteo.lluviaDetectada ? 1 : 0,
                    datosMeteo.velocidadViento,
                    datosMeteo.indiceUV,
                    datosGPS.latitud,
                    datosGPS.longitud
                );
                mqttClient.publish(MQTT_TOPIC_STATUS, payload);
                
                snprintf(payload, sizeof(payload),
                    "{\"vbateria\":%.2f,\"vpanel\":%.2f,\"corriente\":%.3f,\"potencia\":%.2f,\"porcentaje\":%.1f,\"tempBat\":%.1f,\"uptime\":%lu}",
                    datosSolar.voltajeBateria,
                    datosSolar.voltajePanel,
                    datosSolar.corriente,
                    datosSolar.potencia,
                    datosSolar.porcentajeBateria,
                    temperaturaBateria,
                    millis() / 1000
                );
                mqttClient.publish(MQTT_TOPIC_SOLAR, payload);
                
                lastPublish = millis();
            }
            
            static unsigned long lastGPS = 0;
            if (millis() - lastGPS > 300000) {
                if (comunicaciones.obtenerGPS(datosGPS)) {
                    char payload[150];
                    snprintf(payload, sizeof(payload),
                        "{\"lat\":%.6f,\"lon\":%.6f,\"uptime\":%lu}",
                        datosGPS.latitud, datosGPS.longitud, millis() / 1000
                    );
                    mqttClient.publish(MQTT_TOPIC_GPS, payload);
                }
                lastGPS = millis();
            }
        }
    }
    
    if (energia.getModoOperacion() != MODO_MINIMO) {
        actualizarSemaforo();
        actualizarIndicadorUV(datosMeteo.indiceUV);
        
        static unsigned long lastDisplayUpdate = 0;
        if (millis() - lastDisplayUpdate > 3000) {
            static uint8_t displayMode = 0;
            displayMode = (displayMode + 1) % 4;
            
            if (displayMode == 0) {
                mostrarEstadoSemaforo();
            } else if (displayMode == 1) {
                mostrarInfoSolar();
            } else if (displayMode == 2) {
                mostrarInfoAmbiente();
            } else {
                mostrarInfoUV();
            }
            lastDisplayUpdate = millis();
        }
    } else {
        digitalWrite(PIN_LED_RED, LED_OFF);
        digitalWrite(PIN_LED_YELLOW, LED_OFF);
        digitalWrite(PIN_LED_GREEN, LED_OFF);
        digitalWrite(PIN_LED_UV_VERDE, LED_OFF);
        digitalWrite(PIN_LED_UV_AMARILLO, LED_OFF);
        digitalWrite(PIN_LED_UV_NARANJA, LED_OFF);
        digitalWrite(PIN_LED_UV_ROJO, LED_OFF);
        digitalWrite(PIN_LED_UV_VIOLETA, LED_OFF);
        
        static unsigned long lastLowBatteryBlink = 0;
        if (millis() - lastLowBatteryBlink > 2000) {
            if ((millis() / 4000) % 2 == 0) {
                digitalWrite(PIN_LED_RED, LED_ON);
            }
            lastLowBatteryBlink = millis();
        }
    }
    
    static unsigned long lastDebug = 0;
    unsigned long debugInterval = (energia.getModoOperacion() == MODO_MINIMO) ? 30000 : 10000;
    if (millis() - lastDebug > debugInterval) {
        const char* modoStr = (energia.getModoOperacion() == MODO_NORMAL) ? "NORMAL" : 
                             (energia.getModoOperacion() == MODO_AHORRO) ? "AHORRO" : "MINIMO";
        const char* nivelStr = (energia.getNivelBateria() == NIVEL_NORMAL) ? "OK" : 
                              (energia.getNivelBateria() == NIVEL_ADVERTENCIA) ? "WARN" : 
                              (energia.getNivelBateria() == NIVEL_BAJO) ? "BAJA" : "CRIT";
        Serial.printf("[%lu] %s | Bat: %.2fV (%s) T:%.1fC | Modo: %s | Wake: %u | Lluvia: %s | Viento: %.1f m/s\n",
            millis() / 1000,
            mqttConectado ? "MQTT OK" : "MQTT OFF",
            datosSolar.voltajeBateria,
            nivelStr,
            temperaturaBateria,
            modoStr,
            wakeUpCount,
            datosMeteo.lluviaDetectada ? "SI" : "NO",
            datosMeteo.velocidadViento
        );
        lastDebug = millis();
    }
    
    delay(50);
}