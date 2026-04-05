#ifndef METEOROLOGIA_H
#define METEOROLOGIA_H

#include <Arduino.h>
#include <driver/timer.h>

struct DatosMeteorologicos {
    EstadoLluvia lluvia;
    float velocidadViento;
    float indiceUV;
    bool lluviaDetectada;
    bool vientoDetectado;
};

class Meteorologia {
public:
    void init();
    DatosMeteorologicos leerDatos();
    EstadoLluvia leerSensorLluvia();
    float leerSensorViento();
    float leerSensorUV();
    void IRAM_ATTR contarPulsoViento();
    
private:
    volatile uint32_t pulsosViento;
    unsigned long ultimoTiempoViento;
    float calcularVelocidadViento();
};

#endif // METEOROLOGIA_H