#include "energia.h"
#include "config.h"
#include <esp_sleep.h>

void Energia::init() {
    modoActual = MODO_NORMAL;
    nivelActual = NIVEL_NORMAL;
    deepSleepCount = 0;
    tiempoInicio = millis();
    
    stats.whConsumidos = 0.0;
    stats.whGenerados = 0.0;
    stats.eficienciaCarga = 0.0;
    stats.tiempoFuncionamiento = 0;
    stats.ciclosDeepSleep = 0;
    
    cargarDeNVS();
    
    Serial.println("Sistema de energia inicializado");
}

void Energia::cargarDeNVS() {
    preferences.begin("energia_solar", true);
    stats.whConsumidos = preferences.getFloat("whCons", 0.0);
    stats.whGenerados = preferences.getFloat("whGen", 0.0);
    deepSleepCount = preferences.getUInt("dsCount", 0);
    preferences.end();
}

void Energia::guardarEnNVS() {
    preferences.begin("energia_solar", false);
    preferences.putFloat("whCons", stats.whConsumidos);
    preferences.putFloat("whGen", stats.whGenerados);
    preferences.putUInt("dsCount", deepSleepCount);
    preferences.end();
}

void Energia::cargarConfig() {
    preferences.begin("semaforo", true);
    TIEMPO_ROJO = preferences.getFloat("tiempoRojo", 30000);
    TIEMPO_VERDE = preferences.getFloat("tiempoVerde", 25000);
    TIEMPO_AMARILLO = preferences.getFloat("tiempoAmarillo", 5000);
    BATERIA_CRITICA = preferences.getFloat("batCritica", 11.0);
    BATERIA_BAJA = preferences.getFloat("batBaja", 11.5);
    BATERIA_ADVERTENCIA = preferences.getFloat("batAdvertencia", 12.0);
    BATERIA_NORMAL = preferences.getFloat("batNormal", 12.5);
    preferences.end();
    Serial.println("Configuracion cargada desde NVS");
}

void Energia::guardarConfig() {
    preferences.begin("semaforo", false);
    preferences.putFloat("tiempoRojo", TIEMPO_ROJO);
    preferences.putFloat("tiempoVerde", TIEMPO_VERDE);
    preferences.putFloat("tiempoAmarillo", TIEMPO_AMARILLO);
    preferences.putFloat("batCritica", BATERIA_CRITICA);
    preferences.putFloat("batBaja", BATERIA_BAJA);
    preferences.putFloat("batAdvertencia", BATERIA_ADVERTENCIA);
    preferences.putFloat("batNormal", BATERIA_NORMAL);
    preferences.end();
    Serial.println("Configuracion guardada en NVS");
}

NivelBateria Energia::evaluarNivelBateria(float voltaje) {
    if (voltaje < BATERIA_CRITICA) {
        nivelActual = NIVEL_CRITICO;
    } else if (voltaje < BATERIA_BAJA) {
        nivelActual = NIVEL_BAJO;
    } else if (voltaje < BATERIA_ADVERTENCIA) {
        nivelActual = NIVEL_ADVERTENCIA;
    } else {
        nivelActual = NIVEL_NORMAL;
    }
    return nivelActual;
}

bool Energia::evaluarTemperaturaBateria(float tempBateria) {
    if (tempBateria < TEMP_BATERIA_MIN || tempBateria > TEMP_BATERIA_MAX) {
        Serial.printf("ALERTA: Temperatura bateria fuera de rango: %.1fC\n", tempBateria);
        return true;
    }
    return false;
}

void Energia::gestionarBateria(float voltajeBateria, float tempBateria, float potenciaPanel) {
    nivelActual = evaluarNivelBateria(voltajeBateria);
    
    if (evaluarTemperaturaBateria(tempBateria)) {
        if (tempBateria < TEMP_BATERIA_MIN || tempBateria > TEMP_BATERIA_MAX + 10) {
            Serial.println("Temperatura critica, entrando deep sleep...");
            delay(2000);
            entrarDeepSleep(DEEPSLEEP_SEGUNDOS);
        }
    }
    
    if (modoActual == MODO_NORMAL && nivelActual >= NIVEL_BAJO) {
        modoActual = MODO_AHORRO;
        Serial.println("Cambiando a modo AHORRO");
    }
    
    if (modoActual == MODO_AHORRO && nivelActual == NIVEL_NORMAL) {
        if (voltajeBateria >= BATERIA_NORMAL) {
            modoActual = MODO_NORMAL;
            Serial.println(" Recuperando modo NORMAL");
        }
    }
    
    switch (nivelActual) {
        case NIVEL_CRITICO:
            Serial.printf("ALERTA: Bateria CRITICA (%.2fV) - Deep sleep inmediato\n", voltajeBateria);
            delay(1000);
            entrarDeepSleep(DEEPSLEEP_CRITICO_SEGUNDOS);
            break;
            
        case NIVEL_BAJO:
            Serial.printf("ALERTA: Bateria BAJA (%.2fV)\n", voltajeBateria);
            modoActual = MODO_MINIMO;
            if (potenciaPanel < 5.0) {
                Serial.println("Panel sin carga, entrando deep sleep...");
                delay(2000);
                entrarDeepSleep(DEEPSLEEP_SEGUNDOS);
            }
            break;
            
        case NIVEL_ADVERTENCIA:
            Serial.printf("ADVERTENCIA: Bateria baja (%.2fV)\n", voltajeBateria);
            modoActual = MODO_AHORRO;
            break;
            
        case NIVEL_NORMAL:
            modoActual = MODO_NORMAL;
            break;
    }
}

void Energia::entrarDeepSleep(uint32_t segundos) {
    Serial.printf("Entrando en deep sleep por %u segundos...\n", segundos);
    
    deepSleepCount++;
    stats.ciclosDeepSleep = deepSleepCount;
    guardarEnNVS();
    
    esp_sleep_enable_timer_wakeup(segundos * 1000000ULL);
    
    Serial.flush();
    esp_deep_sleep_start();
}

void Energia::actualizarEstadisticas(float corriente, float deltaTime) {
    float potencia = abs(corriente * 12.0);
    float energia = potencia * (deltaTime / 3600.0);
    
    if (corriente > 0) {
        stats.whConsumidos += energia;
    } else {
        stats.whGenerados += energia;
    }
    
    if (stats.whGenerados > 0) {
        stats.eficienciaCarga = (stats.whConsumidos / stats.whGenerados) * 100.0;
    }
    
    stats.tiempoFuncionamiento = millis() - tiempoInicio;
}

EnergiaStats Energia::getEstadisticas() {
    stats.tiempoFuncionamiento = millis() - tiempoInicio;
    return stats;
}

ModoOperacion Energia::getModoOperacion() {
    return modoActual;
}

NivelBateria Energia::getNivelBateria() {
    return nivelActual;
}

bool Energia::esRecuperacionBateria() {
    return (nivelActual == NIVEL_CRITICO || nivelActual == NIVEL_BAJO);
}

void Energia::resetEstadisticas() {
    stats.whConsumidos = 0.0;
    stats.whGenerados = 0.0;
    stats.eficienciaCarga = 0.0;
    stats.ciclosDeepSleep = 0;
    deepSleepCount = 0;
    guardarEnNVS();
    Serial.println("Estadisticas de energia reseteadas");
}