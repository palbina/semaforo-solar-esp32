#ifndef ENERGIA_H
#define ENERGIA_H

#include <Arduino.h>
#include <Preferences.h>

struct EnergiaStats {
    float whConsumidos;
    float whGenerados;
    float eficienciaCarga;
    unsigned long tiempoFuncionamiento;
    uint32_t ciclosDeepSleep;
};

class Energia {
public:
    void init();
    void cargarConfig();
    void guardarConfig();
    NivelBateria evaluarNivelBateria(float voltaje);
    bool evaluarTemperaturaBateria(float tempBateria);
    void gestionarBateria(float voltajeBateria, float tempBateria, float potenciaPanel);
    void entrarDeepSleep(uint32_t segundos);
    void actualizarEstadisticas(float corriente, float deltaTime);
    EnergiaStats getEstadisticas();
    ModoOperacion getModoOperacion();
    NivelBateria getNivelBateria();
    bool esRecuperacionBateria();
    void resetEstadisticas();
    
private:
    Preferences preferences;
    EnergiaStats stats;
    ModoOperacion modoActual;
    NivelBateria nivelActual;
    uint32_t deepSleepCount;
    unsigned long tiempoInicio;
    
    void guardarEnNVS();
    void cargarDeNVS();
};

#endif // ENERGIA_H