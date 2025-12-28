#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <signal.h>

/* ===== Struktura pamięci dzielonej ===== */
typedef struct {
    int ciastka;
    int max;
} shared_data_t;

/* ===== Globalna flaga pracy ===== */
extern volatile sig_atomic_t running;

/* ===== Shared memory ===== */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

/* ===== Procesy ===== */
void proces_piekarz(int id, shared_data_t *shm);
void proces_kasjer(int id);
void proces_klient(int id);

/* ===== SIGINT ===== */
void sigint_handler(int sig);

#endif
