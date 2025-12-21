#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>
#include <signal.h>

/* =======================
 * Dane współdzielone
 * ======================= */
typedef struct {
    int ciastka;     // aktualna liczba ciastek
    int max;         // pojemność magazynu
    int klienci;     // liczba klientów w sklepie
} shared_data_t;

/* =======================
 * Flaga globalna (sygnały)
 * ======================= */
extern volatile sig_atomic_t running;

/* =======================
 * IPC – pamięć dzielona
 * ======================= */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

/* =======================
 * Obsługa sygnałów
 * ======================= */
void sigint_handler(int sig);

/* =======================
 * Procesy symulacji
 * ======================= */
void proces_piekarz(int id, shared_data_t *shm);
void proces_kasjer(int id);
void proces_klient(int id);
void generator_klientow(void);

#endif
