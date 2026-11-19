#include <stdio.h>        // printf
#include "esp_timer.h"    // tempo desde que o ESP32 ligou
#include "comunicacao.h"  // i2cLer
#include "tempo.h"

#define ENDERECO_RTC 0x68    // "endereço" do RTC DS1307 no I2C
#define HORA_ENTRADA 8       // o expediente começa às 08:00
#define HORA_SAIDA 18        // o expediente termina às 18:00
#define AJUSTE_DE_HORAS 0    // use -3 se o LCD mostrar 3 horas a mais (horário de Brasília)

// Última hora lida do RTC. "static" deixa as variáveis só neste arquivo.
static int hora = 0;
static int minuto = 0;
static bool rtcFuncionando = true;  // para avisar do erro uma vez só

// Quantos milissegundos se passaram desde que o ESP32 ligou.
int64_t milissegundos(void) {
  return esp_timer_get_time() / 1000;
}

// Converte o formato do relógio (BCD) em número normal: 0x45 vira 45.
static int bcdParaNumero(uint8_t valor) {
  return (valor / 16) * 10 + (valor % 16);
}

// Lê a hora do RTC. Se a leitura falhar, continua com a última hora lida.
void tempoAtualizar(void) {
  uint8_t dados[2];  // dados[0] = minutos, dados[1] = horas

  if (!i2cLer(ENDERECO_RTC, 0x01, dados, sizeof(dados))) {  // 0x01: registrador dos minutos
    if (rtcFuncionando) {
      printf("ERRO: RTC nao respondeu\n");
      rtcFuncionando = false;
    }
    return;
  }

  rtcFuncionando = true;
  minuto = bcdParaNumero(dados[0]);
  hora = (bcdParaNumero(dados[1] & 0x3F) + AJUSTE_DE_HORAS + 24) % 24;  // formato de 24 horas
}

int horaAtual(void) {
  return hora;
}

int minutoAtual(void) {
  return minuto;
}

// Responde true se agora estamos fora do horário de trabalho.
bool foraDoHorario(void) {
  return hora < HORA_ENTRADA || hora >= HORA_SAIDA;
}
