#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>

// Funkcje dla magazynu i piekarza
void init_magazyn(int pojemnosc);
void piekarz_proc(int id);
void cleanup_magazyn();

#endif // CIASTKARNIA_H
