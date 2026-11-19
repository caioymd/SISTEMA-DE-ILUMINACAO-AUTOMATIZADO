#ifndef COMUNICACAO_H  // evita que este arquivo seja incluído duas vezes
#define COMUNICACAO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void comunicacaoIniciar(void);

// I2C: lê dados de um periférico (usado para ler o RTC). Responde true se deu certo.
bool i2cLer(uint8_t endereco, uint8_t registrador, uint8_t *dados, size_t tamanho);

// LCD: mostra o horário do último movimento e se foi fora do expediente.
void mostrarHorarioNoLcd(int hora, int minuto, bool foraDoExpediente);

// Monitor Serial: escreve o horário atual seguido de uma mensagem.
void enviarMensagem(const char *mensagem);

#endif
