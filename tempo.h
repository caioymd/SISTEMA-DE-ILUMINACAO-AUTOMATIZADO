#ifndef TEMPO_H  // evita que este arquivo seja incluído duas vezes
#define TEMPO_H

#include <stdbool.h>
#include <stdint.h>

int64_t milissegundos(void);
void tempoAtualizar(void);
int horaAtual(void);
int minutoAtual(void);
bool foraDoHorario(void);

#endif
