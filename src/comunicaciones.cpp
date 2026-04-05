#include "comunicaciones.h"
#include "config.h"

extern HardwareSerial SerialAT;
extern TinyGsm modem;

void Comunicaciones::init() {
    wifiConectado = false;
    gsmConectado = false;
    loraIniciado = false;
    mqttConectado = false;
    conexionActiva = CONEXION_GSM;
    
    Serial.println("Sistema de comunicaciones inicializado");
}

bool Comunicaciones::iniciarWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConectado = true;
        conexionActiva = CONEXION_WIFI;
        Serial.println("WiFi conectado: " + WiFi.localIP().toString());
        return true;
    }
    
    Serial.println("WiFi no conectado");
    return false;
}

bool Comunicaciones::iniciarLoRa() {
    LoRa.setPins(PIN_LORA_CS, PIN_LORA_RST, PIN_LORA_DIO0);
    
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("ERROR: LoRa no inicializado");
        loraIniciado = false;
        return false;
    }
    
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.setTxPower(20);
    
    loraIniciado = true;
    Serial.println("LoRa inicializado en " + String(LORA_FREQUENCY / 1000000) + "MHz");
    return true;
}

bool Comunicaciones::iniciarModem() {
    SerialAT.begin(115200, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
    
    pinMode(PIN_PWRKEY, OUTPUT);
    digitalWrite(PIN_PWRKEY, LOW);
    delay(100);
    digitalWrite(PIN_PWRKEY, HIGH);
    delay(1000);
    digitalWrite(PIN_PWRKEY, LOW);
    
    Serial.println("Esperando modem SIM7000G...");
    
    int retry = 0;
    while (!modem.testAT(1000) && retry < 30) {
        Serial.print(".");
        retry++;
        delay(500);
    }
    Serial.println();
    
    if (retry >= 30) {
        Serial.println("ERROR: No se detecto el modem");
        return false;
    }
    
    Serial.println("Modem detectado, inicializando...");
    modem.init();
    
    String modelo = modem.getModemName();
    Serial.println("Modem: " + modelo);
    
    Serial.println("Esperando senal GSM...");
    retry = 0;
    while (modem.getSignalQuality() < 5 && retry < 60) {
        delay(1000);
        retry++;
        Serial.print(".");
    }
    Serial.println();
    
    int signal = modem.getSignalQuality();
    Serial.printf("Nivel de senal: %d/31\n", signal);
    
    if (signal < 5) {
        Serial.println("ERROR: Senal muy debil");
        return false;
    }
    
    gsmConectado = true;
    conexionActiva = CONEXION_GSM;
    Serial.println("Modem GSM conectado correctamente");
    return true;
}

bool Comunicaciones::conectarGPRS() {
    Serial.println("Configurando GPRS...");
    
    modem.sendAT("+COPS=0");
    delay(2000);
    
    modem.sendAT("+CNMP=38");
    delay(1000);
    
    const char* apns[] = {"internet.claro.com.ar", "wap.claro.com.ar", "igprs.claro"};
    
    for (int i = 0; i < 3; i++) {
        Serial.printf("Intentando APN: %s...\n", apns[i]);
        if (modem.gprsConnect(apns[i])) {
            Serial.println("GPRS conectado");
            return true;
        }
        
        if (i < 2) {
            int espera = 1000 * pow(2, i);
            Serial.printf("Reintentando en %d ms...\n", espera);
            delay(espera);
        }
    }
    
    Serial.println("ERROR: No se pudo conectar GPRS");
    return false;
}

bool Comunicaciones::conectarMQTT(PubSubClient& client, TinyGsmClient& gsmClient) {
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->callbackMQTT(topic, payload, length);
    });
    
    Serial.print("Conectando a MQTT broker...");
    
    int intento = 0;
    while (!client.connect(MQTT_CLIENT_ID) && intento < 5) {
        Serial.print(".");
        delay(min(1000 * pow(2, intento), 5000));
        intento++;
    }
    Serial.println();
    
    if (!client.connected()) {
        Serial.println("ERROR: Fallo conexion MQTT");
        return false;
    }
    
    client.subscribe(MQTT_TOPIC_CONTROL);
    mqttConectado = true;
    Serial.println("MQTT conectado y suscripto a: " + String(MQTT_TOPIC_CONTROL));
    return true;
}

void Comunicaciones::publicarEstado(PubSubClient& client, const char* payload) {
    if (mqttConectado) {
        client.publish(MQTT_TOPIC_STATUS, payload);
    }
}

void Comunicaciones::publicarDatosSolar(PubSubClient& client, const char* payload) {
    if (mqttConectado) {
        client.publish(MQTT_TOPIC_SOLAR, payload);
    }
}

void Comunicaciones::publicarAlerta(PubSubClient& client, const char* tipo, const char* mensaje) {
    if (mqttConectado) {
        char payload[200];
        snprintf(payload, sizeof(payload), 
            "{\"tipo\":\"%s\",\"mensaje\":\"%s\"}", tipo, mensaje);
        client.publish(MQTT_TOPIC_ALERTAS, payload);
    }
}

void Comunicaciones::enviarLoRa(const uint8_t* data, size_t len) {
    if (!loraIniciado) return;
    
    LoRa.beginPacket();
    LoRa.write(data, len);
    LoRa.endPacket();
}

bool Comunicaciones::estaConectadoWiFi() {
    return wifiConectado && WiFi.status() == WL_CONNECTED;
}

bool Comunicaciones::estaConectadoGSM() {
    return gsmConectado;
}

bool Comunicaciones::estaConectadoLoRa() {
    return loraIniciado;
}

int Comunicaciones::obtenerSignalQuality() {
    if (gsmConectado) {
        return modem.getSignalQuality();
    }
    return 0;
}

bool Comunicaciones::obtenerGPS(DatosGPS& gps) {
    if (!gsmConectado) {
        gps.valido = false;
        return false;
    }
    
    Serial.println("Obteniendo GPS...");
    
    modem.sendAT("+CGNSSPWR=1");
    delay(500);
    
    int retry = 0;
    while (retry < 10) {
        modem.sendAT("+CGNSINF");
        
        String response = "";
        if (modem.waitResponse(1000, response)) {
            if (response.indexOf("+CGNSINF:") != -1 && response.indexOf(",1,") != -1) {
                int firstComma = response.indexOf(",");
                int secondComma = response.indexOf(",", firstComma + 1);
                int thirdComma = response.indexOf(",", secondComma + 1);
                
                if (firstComma > 0 && secondComma > 0 && thirdComma > 0) {
                    gps.latitud = response.substring(firstComma + 1, secondComma).toFloat();
                    gps.longitud = response.substring(secondComma + 1, thirdComma).toFloat();
                    gps.valido = true;
                    
                    Serial.printf("GPS: %.6f, %.6f\n", gps.latitud, gps.longitud);
                    return true;
                }
            }
        }
        retry++;
        delay(1000);
    }
    
    gps.valido = false;
    return false;
}

void Comunicaciones::callbackMQTT(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje MQTT en topic: ");
    Serial.println(topic);
    
    char mensaje[length + 1];
    for (unsigned int i = 0; i < length; i++) {
        mensaje[i] = (char)payload[i];
    }
    mensaje[length] = '\0';
    
    Serial.println("Mensaje: " + String(mensaje));
}

TipoConexion Comunicaciones::getConexionActiva() {
    return conexionActiva;
}