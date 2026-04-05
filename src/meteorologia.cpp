#include "meteorologia.h"
#include "config.h"

volatile uint32_t contadorPulsos = 0;
unsigned long ultimoTiempoViento = 0;

void IRAM_ATTR contarPulsoVientoISR() {
    contadorPulsos++;
}

void Meteorologia::init() {
    pinMode(PIN_SENSOR_LLUVIA, INPUT);
    pinMode(PIN_SENSOR_Viento, INPUT_PULLUP);
    
    attachInterrupt(PIN_SENSOR_Viento, contarPulsoVientoISR, FALLING);
    
    Serial.println("Sensores meteorologicos inicializados");
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
    
    float revoluciones = (float)pulsos / 1;
    float rpm = (revoluciones * 60000.0) / tiempoTranscurrido;
    
    float velocidad = (rpm * 9.0 * 0.014) / 60.0;
    
    return velocidad;
}

float Meteorologia::leerSensorUV() {
    int rawADC = analogRead(39);
    float voltaje = (rawADC * 3.3) / 4095;
    
    float indiceUV = (voltaje / 3.3) * 11.0;
    
    return constrain(indiceUV, 0.0, 11.0);
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
