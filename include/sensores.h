#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>
#include <Adafruit_INA219.h>

struct DatosSolar {
    float voltajeBateria;
    float voltajePanel;
    float corriente;
    float potencia;
    float porcentajeBateria;
    float shuntVoltage;
    float busVoltage;
};

struct DatosAmbiente {
    float temperatura;
    float humedad;
};

struct DatosINA219 {
    float busVoltage;
    float shuntVoltage;
    float current;
    float power;
    float loadVoltage;
};

class Sensores {
public:
    void init();
    DatosSolar leerDatosSolar();
    DatosAmbiente leerDatosAmbiente();
    DatosINA219 leerINA219();
    float leerVoltajeBateriaOriginal();
    float leerVoltajePanel();
    float leerNTCTemperatura();
    float calcularPorcentajeBateria(float voltaje);
    
private:
    Adafruit_INA219 ina219;
    int analogReadPromedio(int pin);
};

#endif // SENSORES_H