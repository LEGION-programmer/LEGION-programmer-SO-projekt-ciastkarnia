#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>

// =======================
// Struktury danych
// =======================
typedef struct {
    int ciastka;    // liczba ciastek w magazynie
    int max;        // pojemność magazynu
    int klienci;    // liczba klientów w sklepie
} shared_data_t;

// =======================
// Funkcje IPC / magazyn
// =======================
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

#endif
