#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>
#include <signal.h>   // <-- TO MUSI TU BYĆ

typedef struct {
    int ciastka;
    int max;
    int klienci;
} shared_data_t;

// shared memory
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

// semafory
int init_semaphore();
void sem_lock(int semid);
void sem_unlock(int semid);

// SIGINT
extern volatile sig_atomic_t running;
void sigint_handler(int sig);

#endif
