#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    (void)sig;
    running = 0;
}

/* ===== PAMIĘĆ DZIELONA ===== */

void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0660);
    if (*shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    *shm = shmat(*shm_id, NULL, 0);
    if (*shm == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    (*shm)->ciastka = 0;
    (*shm)->max = max_magazyn;
}

void cleanup_shared_memory(int shm_id, shared_data_t *shm) {
    shmdt(shm);
    shmctl(shm_id, IPC_RMID, NULL);
}

/* ===== SEMAFOR ===== */

int init_semaphore() {
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0660);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    semctl(semid, 0, SETVAL, 1);
    return semid;
}

void cleanup_semaphore(int semid) {
    semctl(semid, 0, IPC_RMID);
}
