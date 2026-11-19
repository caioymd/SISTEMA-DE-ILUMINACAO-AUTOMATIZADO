#include <stdio.h>             // printf e snprintf
#include <stdint.h>            // tipos como uint8_t (número de 8 bits)
#include "freertos/FreeRTOS.h" // sistema que controla as tarefas do ESP32
#include "freertos/task.h"     // vTaskDelay (esperar um tempo)
#include "driver/i2c.h"        // comunicação I2C
#include "comunicacao.h"
#include "tempo.h"             // horaAtual e minutoAtual

#define PINO_SDA 21             // fio de dados do I2C
#define PINO_SCL 22             // fio de clock do I2C
#define PORTA_I2C I2C_NUM_0     // o ESP32 tem 2 portas I2C; usamos a primeira
#define VELOCIDADE_I2C 100000   // 100 kHz: velocidade máxima do RTC DS1307
#define ENDERECO_LCD 0x27       // "endereço" do LCD no I2C


/* ----------------------------------------------------------------------
   LCD 16x2 (mostra o horário do último movimento)
   O LCD é ligado por um módulo I2C. Cada byte que ele recebe controla 8 fios:
     - bits 4 a 7: dados (só cabem 4 bits por vez)
     - bit 3: luz de fundo
     - bit 2: "enable" (um pulso avisa o LCD para ler os dados)
     - bit 0: tipo do byte (0 = comando, 1 = letra)

   Como só cabem 4 bits por vez, cada byte é enviado em duas metades.
   ---------------------------------------------------------------------- */

#define LCD_LUZ_DE_FUNDO 0x08
#define LCD_ENABLE 0x04
#define LCD_COMANDO 0x00
#define LCD_LETRA 0x01
#define LCD_LIMPAR 0x01       // comando: apaga a tela
#define LCD_LINHA_1 0x80      // comando: vai para o início da linha de cima
#define LCD_LINHA_2 0xC0      // comando: vai para o início da linha de baixo

// Envia 4 bits para o LCD, dando o pulso no "enable" para ele ler.
static void lcdEnviarMetade(uint8_t metade, uint8_t tipo) {
  uint8_t base = (metade << 4) | LCD_LUZ_DE_FUNDO | tipo;
  uint8_t pulso[] = { base | LCD_ENABLE, base };  // enable ligado e desligado

  esp_err_t resultado = i2c_master_write_to_device(PORTA_I2C, ENDERECO_LCD,
                                                   pulso, sizeof(pulso),
                                                   pdMS_TO_TICKS(1000));
  if (resultado != ESP_OK) {
    printf("ERRO: LCD nao respondeu\n");
  }
}

// Envia um byte inteiro: primeiro os 4 bits altos e depois os 4 bits baixos.
static void lcdEnviarByte(uint8_t valor, uint8_t tipo) {
  lcdEnviarMetade(valor >> 4, tipo);
  lcdEnviarMetade(valor & 0x0F, tipo);
}

// Escreve um texto a partir de onde o LCD está posicionado.
static void lcdEscrever(const char *texto) {
  for (int i = 0; texto[i] != '\0'; i++) {  // '\0' marca o fim do texto
    lcdEnviarByte(texto[i], LCD_LETRA);
  }
}

// Apaga a tela e escreve as duas linhas.
static void lcdMostrar(const char *linha1, const char *linha2) {
  lcdEnviarByte(LCD_LIMPAR, LCD_COMANDO);
  vTaskDelay(pdMS_TO_TICKS(10));  // o LCD demora um pouco para apagar
  lcdEnviarByte(LCD_LINHA_1, LCD_COMANDO);
  lcdEscrever(linha1);
  lcdEnviarByte(LCD_LINHA_2, LCD_COMANDO);
  lcdEscrever(linha2);
}

// Mostra no LCD o horário do movimento e se ele foi fora do expediente.
void mostrarHorarioNoLcd(int hora, int minuto, bool foraDoExpediente) {
  char linha2[32];
  snprintf(linha2, sizeof(linha2), "Horario: %02d:%02d", hora, minuto);
  lcdMostrar(foraDoExpediente ? "Fora do horario" : "Ultimo movimento", linha2);
}

// Prepara o LCD (sequência definida pelo chip HD44780, usado neste LCD).
static void lcdIniciar(void) {
  vTaskDelay(pdMS_TO_TICKS(50));  // espera o LCD ligar

  // Estas metades colocam o LCD no modo de 4 bits.
  lcdEnviarMetade(0x03, LCD_COMANDO);
  vTaskDelay(pdMS_TO_TICKS(10));
  lcdEnviarMetade(0x03, LCD_COMANDO);
  vTaskDelay(pdMS_TO_TICKS(10));
  lcdEnviarMetade(0x03, LCD_COMANDO);
  vTaskDelay(pdMS_TO_TICKS(10));
  lcdEnviarMetade(0x02, LCD_COMANDO);

  lcdEnviarByte(0x28, LCD_COMANDO);  // 2 linhas, letras de 5x8 pixels
  lcdEnviarByte(0x0C, LCD_COMANDO);  // liga a tela, sem cursor
  lcdEnviarByte(0x06, LCD_COMANDO);  // a cada letra, anda para a direita

  lcdMostrar("Aguardando", "movimento...");
}


/* ----------------------------------------------------------------------
   INICIALIZAÇÃO, LEITURA DO I2C E MONITOR SERIAL
   ---------------------------------------------------------------------- */

// Liga o I2C e o LCD. Roda uma vez, quando o ESP32 inicia.
void comunicacaoIniciar(void) {
  // Configura o ESP32 como mestre do I2C: é ele quem controla a conversa.
  i2c_config_t configuracao = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = PINO_SDA,
    .scl_io_num = PINO_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = VELOCIDADE_I2C
  };
  i2c_param_config(PORTA_I2C, &configuracao);
  i2c_driver_install(PORTA_I2C, I2C_MODE_MASTER, 0, 0, 0);

  lcdIniciar();
}

// Lê dados de um periférico a partir de um registrador. Responde true se deu certo.
bool i2cLer(uint8_t endereco, uint8_t registrador, uint8_t *dados, size_t tamanho) {
  esp_err_t resultado = i2c_master_write_read_device(PORTA_I2C, endereco,
                                                     &registrador, 1,
                                                     dados, tamanho,
                                                     pdMS_TO_TICKS(1000));
  return resultado == ESP_OK;
}

// Escreve no Monitor Serial o horário atual seguido de uma mensagem.
void enviarMensagem(const char *mensagem) {
  printf("[%02d:%02d] %s\n", horaAtual(), minutoAtual(), mensagem);
}
