#include "driver/gpio.h"  // ler e escrever nos pinos
#include "led.h"

#define PINO_LED GPIO_NUM_23
#define PINO_MODO_1 GPIO_NUM_25
#define PINO_MODO_2 GPIO_NUM_26

// Guarda se a luz está ligada. "static" deixa a variável só neste arquivo.
static bool luzLigada = false;

void ledLigar(void) {
  gpio_set_level(PINO_LED, 1);
  luzLigada = true;
}

void ledDesligar(void) {
  gpio_set_level(PINO_LED, 0);
  luzLigada = false;
}

// Responde true se a luz está ligada neste momento.
bool ledEstaLigado(void) {
  return luzLigada;
}

// Acende somente o LED correspondente ao modo.
void ledMostrarModo(bool foraDoExpediente) {
  gpio_set_level(PINO_MODO_1, !foraDoExpediente);
  gpio_set_level(PINO_MODO_2, foraDoExpediente);
}

// Configura os pinos dos LEDs como saída e começa com a luz apagada.
void ledIniciar(void) {
  gpio_reset_pin(PINO_LED);
  gpio_set_direction(PINO_LED, GPIO_MODE_OUTPUT);
  gpio_reset_pin(PINO_MODO_1);
  gpio_set_direction(PINO_MODO_1, GPIO_MODE_OUTPUT);
  gpio_reset_pin(PINO_MODO_2);
  gpio_set_direction(PINO_MODO_2, GPIO_MODE_OUTPUT);
  ledDesligar();
}
