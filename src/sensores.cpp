#include "sensores.h"
#include "config.h"
#include <DHT.h>

extern DHT dht;

void Sensores::init() {
    if (!ina219.begin()) {
        Serial.println("ERROR: No se encontró INA219");
    } else {
        ina219.setCalibration(0x1000, 0x4000);
        Serial.println("INA219 inicializado");
    }
    
    pinMode(PIN_ADC_BATERIA, INPUT);
    pinMode(PIN_ADC_PANEL, INPUT);
    pinMode(PIN_NTC_BATERIA, INPUT);
    
    analogSetAttenuation(ADC_11db);
}

int Sensores::analogReadPromedio(int pin) {
    long suma = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        suma += analogRead(pin);
        delayMicroseconds(100);
    }
    return suma / ADC_SAMPLES;
}

DatosINA219 Sensores::leerINA219() {
    DatosINA219 datos;
    
    datos.busVoltage = ina219.getBusVoltage_V();
    datos.shuntVoltage = ina219.getShuntVoltage_mV() / 1000.0;
    datos.current = ina219.getCurrent_mA() / 1000.0;
    datos.power = ina219.getPower_mW() / 1000.0;
    datos.loadVoltage = datos.busVoltage + datos.shuntVoltage;
    
    return datos;
}

float Sensores::leerVoltajeBateriaOriginal() {
    int rawADC = analogReadPromedio(PIN_ADC_BATERIA);
    float voltajeADC = (rawADC * VOLTAJE_REFERENCIA) / ADC_RESOLUTION;
    return voltajeADC * FACTOR_BATERIA;
}

float Sensores::leerVoltajePanel() {
    int rawADC = analogReadPromedio(PIN_ADC_PANEL);
    float voltajeADC = (rawADC * VOLTAJE_REFERENCIA) / ADC_RESOLUTION;
    return voltajeADC * FACTOR_PANEL;
}

float Sensores::leerNTCTemperatura() {
    int rawADC = analogReadPromedio(PIN_NTC_BATERIA);
    float voltaje = (rawADC * VOLTAJE_REFERENCIA) / ADC_RESOLUTION;
    float resistencia = NTC_SERIE_R * (voltaje / (VOLTAJE_REFERENCIA - voltaje));
    float temperatura = (1.0 / ((log(resistencia / NTC_REF_RES) / NTC_BETA) + (1.0 / (NTC_REF_TEMP + 273.15)))) - 273.15;
    return temperatura;
}

float Sensores::calcularPorcentajeBateria(float voltaje) {
    if (voltaje >= 13.0) return 100.0;
    if (voltaje <= 10.5) return 0.0;
    return ((voltaje - 10.5) / (13.0 - 10.5)) * 100.0;
}

DatosSolar Sensores::leerDatosSolar() {
    DatosSolar datos;
    
    DatosINA219 ina = leerINA219();
    datos.voltajeBateria = ina.loadVoltage;
    datos.corriente = ina.current;
    datos.potencia = ina.power;
    datos.shuntVoltage = ina.shuntVoltage;
    datos.busVoltage = ina.busVoltage;
    
    datos.voltajePanel = leerVoltajePanel();
    datos.porcentajeBateria = calcularPorcentajeBateria(datos.voltajeBateria);
    
    return datos;
}

DatosAmbiente Sensores::leerDatosAmbiente() {
    DatosAmbiente datos;
    
    datos.temperatura = dht.readTemperature();
    datos.humedad = dht.readHumidity();
    
    if (isnan(datos.temperatura)) {
        datos.temperatura = 0;
    }
    if (isnan(datos.humedad)) {
        datos.humedad = 0;
    }
    
    return datos;
}