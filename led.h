#ifndef LED_H  // evita que este arquivo seja incluído duas vezes
#define LED_H

#include <stdbool.h>

void ledIniciar(void);
void ledLigar(void);
void ledDesligar(void);
bool ledEstaLigado(void);
void ledMostrarModo(bool foraDoExpediente);

#endif
