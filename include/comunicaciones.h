#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

#include <Arduino.h>
#include <WiFi.h>
#include <LoRa.h>
#include <PubSubClient.h>
#include <TinyGsmClient.h>
#include "config.h"

struct DatosGPS {
    float latitud;
    float longitud;
    bool valido;
};

class Comunicaciones {
public:
    void init();
    bool iniciarWiFi();
    bool iniciarLoRa();
    bool iniciarModem();
    bool conectarGPRS();
    bool conectarMQTT(PubSubClient& client, TinyGsmClient& gsmClient);
    void publicarEstado(PubSubClient& client, const char* payload);
    void publicarDatosSolar(PubSubClient& client, const char* payload);
    void publicarAlerta(PubSubClient& client, const char* tipo, const char* mensaje);
    void enviarLoRa(const uint8_t* data, size_t len);
    bool estaConectadoWiFi();
    bool estaConectadoGSM();
    bool estaConectadoLoRa();
    int obtenerSignalQuality();
    bool obtenerGPS(DatosGPS& gps);
    void callbackMQTT(char* topic, byte* payload, unsigned int length);
    TipoConexion getConexionActiva();
    
private:
    TipoConexion conexionActiva;
    bool wifiConectado;
    bool gsmConectado;
    bool loraIniciado;
    bool mqttConectado;
};

#endif // COMUNICACIONES_H