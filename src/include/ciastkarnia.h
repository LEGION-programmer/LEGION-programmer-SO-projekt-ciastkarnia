#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#define MAX_KLIENCI 10  // Zmniejszyłem dla czytelności testów
#define MAX_MAGAZYN 10

typedef struct {
    int magazyn;
    int klienci_przyszli;
    int klienci_obsluzeni;
    int sprzedane_ciastka;
    pthread_mutex_t mutex; // Mechanizm synchronizacji
} shared_data_t;

/* Procesy */
void proces_piekarz(int id, shared_data_t *shm);
void proces_kasjer(int id, shared_data_t *shm);
void proces_klient(int id, shared_data_t *shm, int ile);

/* IPC Helpers */
shared_data_t* init_shared_memory();
void cleanup_shared_memory(shared_data_t *shm);
void print_stats(shared_data_t *shm);

extern volatile sig_atomic_t running;

#endif