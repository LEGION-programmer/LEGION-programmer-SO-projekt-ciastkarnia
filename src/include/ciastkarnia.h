#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>

/* ===== Pamięć dzielona ===== */
typedef struct {
    int ciastka;
    int max;
} shared_data_t;

/* ===== IPC ===== */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

int init_semaphores();
void sem_down(int semid, int semnum);
void sem_up(int semid, int semnum);

/* ===== Procesy ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid);
void proces_kasjer(int id, shared_data_t *shm, int semid);
void proces_klient(int id, shared_data_t *shm, int semid);

/* ===== Sygnały ===== */
void sigint_handler(int sig);

#endif
