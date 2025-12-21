#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>
#include <signal.h>

/* Globalna flaga sterująca */
extern volatile sig_atomic_t running;

/* Struktura pamięci dzielonej */
typedef struct {
    int ciastka;
    int max;
    int klienci;
} shared_data_t;

/* Pamięć dzielona */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

/* Sygnały */
void sigint_handler(int sig);

/* Procesy */
void proces_piekarz(int id, shared_data_t *shm);
void proces_kasjer(int id);
void proces_klient(int id);

#endif
