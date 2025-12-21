#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <sys/types.h>
#include <signal.h>

extern volatile sig_atomic_t running;

// ===== Shared Memory =====
typedef struct {
    int ciastka;
    int max;
} shared_data_t;

// ===== Message Queue =====
typedef struct {
    long mtype;
    int client_id;
} client_msg_t;

// ===== IPC =====
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

// ===== Semafory =====
int init_semaphore(void);
void semaphore_lock(int semid);
void semaphore_unlock(int semid);
void remove_semaphore(int semid);

// ===== Kolejka =====
int init_queue(void);
void remove_queue(int msgid);

// ===== Procesy =====
void proces_piekarz(int id, shared_data_t *shm, int semid);
void proces_kasjer(int id, int msgid);
void proces_klient(int id, int msgid);

// ===== Sygnały =====
void sigint_handler(int sig);

#endif
