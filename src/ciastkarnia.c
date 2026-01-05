#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdlib.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    (void)sig;
    running = 0;
}

/* ===== SHM ===== */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn, int klienci) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0660);
    *shm = shmat(*shm_id, NULL, 0);

    (*shm)->ciastka = 0;
    (*shm)->max = max_magazyn;
    (*shm)->klienci_pozostali = klienci;
}

void cleanup_shared_memory(int shm_id, shared_data_t *shm) {
    shmdt(shm);
    shmctl(shm_id, IPC_RMID, NULL);
}

/* ===== SEMAFORY =====
   0 – mutex
   1 – limit klientów
*/
int init_semaphores(int max_klientow) {
    int semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0660);
    semctl(semid, 0, SETVAL, 1);
    semctl(semid, 1, SETVAL, max_klientow);
    return semid;
}

void sem_down(int semid, int num) {
    struct sembuf sb = {num, -1, 0};
    semop(semid, &sb, 1);
}

void sem_up(int semid, int num) {
    struct sembuf sb = {num, 1, 0};
    semop(semid, &sb, 1);
}

/* ===== KOLEJKA ===== */
int init_queue() {
    return msgget(IPC_PRIVATE, IPC_CREAT | 0660);
}
