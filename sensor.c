#include "driver/gpio.h"  // ler e escrever nos pinos
#include "sensor.h"

#define PINO_SENSOR GPIO_NUM_27

// Configura o pino do sensor como entrada: o ESP32 vai LER esse pino.
void sensorIniciar(void) {
  gpio_reset_pin(PINO_SENSOR);
  gpio_set_direction(PINO_SENSOR, GPIO_MODE_INPUT);
}

// Responde true se existe movimento agora, ou false se não existe.
bool movimentoDetectado(void) {
  return gpio_get_level(PINO_SENSOR) == 1;
}
