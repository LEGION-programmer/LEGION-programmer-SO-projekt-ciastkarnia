#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    running = 0;
}

/* ===== PAMIĘĆ DZIELONA ===== */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0660);
    if (*shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    *shm = shmat(*shm_id, NULL, 0);
    if (*shm == (void *)-1) {
        perror("shmat");
        exit(1);
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
        exit(1);
    }
    semctl(semid, 0, SETVAL, 1);
    return semid;
}

void sem_down(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_up(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}
