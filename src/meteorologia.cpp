#include "meteorologia.h"
#include "config.h"

volatile uint32_t contadorPulsos = 0;
unsigned long ultimoTiempoViento = 0;

void IRAM_ATTR contarPulsoVientoISR() {
    contadorPulsos++;
}

void Meteorologia::init() {
    pinMode(PIN_SENSOR_LLUVIA, INPUT);
    pinMode(PIN_SENSOR_VIENTo, INPUT_PULLUP);
    
    attachInterrupt(PIN_SENSOR_VIENTo, contarPulsoVientoISR, FALLING);
    
    Serial.println("Sensores meteorológicos inicializados");
}

EstadoLluvia Meteorologia::leerSensorLluvia() {
    int valorDigital = digitalRead(PIN_SENSOR_LLUVIA);
    
    if (valorDigital == LOW) {
        return LLUVIA_LLUVIA;
    }
    
    return LLUVIA_SECO;
}

float Meteorologia::leerSensorViento() {
    uint32_t pulsos = contadorPulsos;
    contadorPulsos = 0;
    
    unsigned long tiempoTranscurrido = millis() - ultimoTiempoViento;
    ultimoTiempoViento = millis();
    
    if (tiempoTranscurrido == 0) return 0.0;
    
    float revoluciones = (float)pulsos / VIENTO_PULSOS_REVOLUCION;
    float rpm = (revoluciones * 60000.0) / tiempoTranscurrido;
    
    float velocidad = (rpm * DIAMETRO_ANEMOMETRO_CM * 0.014) / 60.0;
    
    return velocidad;
}

float Meteorologia::leerSensorUV() {
    int rawADC = analogRead(PIN_SENSOR_UV);
    float voltaje = (rawADC * VOLTAJE_REFERENCIA) / ADC_RESOLUTION;
    
    float indiceUV = (voltaje / UV_MAX_VOLTAGE) * UV_MAX_INDEX;
    
    return constrain(indiceUV, 0.0, UV_MAX_INDEX);
}

DatosMeteorologicos Meteorologia::leerDatos() {
    DatosMeteorologicos datos;
    
    datos.lluvia = leerSensorLluvia();
    datos.velocidadViento = leerSensorViento();
    datos.indiceUV = leerSensorUV();
    datos.lluviaDetectada = (datos.lluvia != LLUVIA_SECO);
    datos.vientoDetectado = (datos.velocidadViento > 0.5);
    
    return datos;
}