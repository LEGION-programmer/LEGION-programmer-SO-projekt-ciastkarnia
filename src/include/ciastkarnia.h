#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>
#include <signal.h>

typedef struct {
    int ciastka;
    int max;
} shared_data_t;

/* IPC */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

int init_semaphore();
void cleanup_semaphore(int semid);

/* Procesy */
void proces_piekarz(int id, shared_data_t *shm, int semid);
void proces_kasjer(int id, shared_data_t *shm, int semid);
void proces_klient(int id, shared_data_t *shm, int semid);

/* SIGINT */
extern volatile sig_atomic_t running;
void sigint_handler(int sig);

#endif
