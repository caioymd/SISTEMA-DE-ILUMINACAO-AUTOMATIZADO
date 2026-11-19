#include <stdbool.h>           // tipo bool (true e false)
#include <stdint.h>            // tipo int64_t
#include "freertos/FreeRTOS.h" // sistema que controla as tarefas do ESP32
#include "freertos/task.h"     // vTaskDelay (esperar um tempo)
#include "comunicacao.h"
#include "sensor.h"
#include "tempo.h"
#include "led.h"

#define TEMPO_LUZ_LIGADA 5000  // ms que a luz fica acesa após o último movimento

// No ESP-IDF o programa começa por aqui: prepara tudo e depois repete para sempre.
void app_main(void) {
  int64_t ultimoMovimento = 0;  // momento (em ms) do último movimento
  bool haviaMovimento = false;  // usado para perceber quando um movimento começa

  // Prepara tudo uma vez. A comunicação vem primeiro porque o RTC usa o I2C.
  comunicacaoIniciar();
  sensorIniciar();
  ledIniciar();

  tempoAtualizar();
  bool estavaForaDoHorario = foraDoHorario();  // para perceber a troca de período
  ledMostrarModo(estavaForaDoHorario);
  enviarMensagem(estavaForaDoHorario ? "Sistema iniciado no Modo 2" : "Sistema iniciado no Modo 1");

  // Repete para sempre.
  while (true) {
    tempoAtualizar();  // lê a hora do RTC uma vez a cada repetição
    bool haMovimento = movimentoDetectado();
    bool foraAgora = foraDoHorario();

    // 1) Avisa quando o expediente começa ou termina e troca o LED de modo.
    if (foraAgora != estavaForaDoHorario) {
      estavaForaDoHorario = foraAgora;
      ledMostrarModo(foraAgora);
      enviarMensagem(foraAgora ? "Fim do expediente: Modo 2" : "Inicio do expediente: Modo 1");
    }

    // 2) Se um movimento acabou de começar, mostra o horário dele no LCD.
    if (haMovimento && !haviaMovimento) {
      mostrarHorarioNoLcd(horaAtual(), minutoAtual(), foraAgora);
      enviarMensagem(foraAgora ? "Movimento fora do horario" : "Movimento no horario de trabalho");
    }

    // 3) Se há movimento, liga a luz.
    if (haMovimento) {
      ultimoMovimento = milissegundos();  // anota quando o movimento aconteceu

      if (!ledEstaLigado()) {
        ledLigar();
        enviarMensagem("Luz ligada");
      }
    }

    // 4) Se a luz está ligada e passou o tempo sem movimento, desliga a luz.
    if (ledEstaLigado() && milissegundos() - ultimoMovimento >= TEMPO_LUZ_LIGADA) {
      ledDesligar();
      enviarMensagem("Sem movimento: luz desligada");
    }

    haviaMovimento = haMovimento;    // guarda para comparar na próxima repetição
    vTaskDelay(pdMS_TO_TICKS(100));  // espera 100 ms antes de verificar tudo de novo
  }
}
