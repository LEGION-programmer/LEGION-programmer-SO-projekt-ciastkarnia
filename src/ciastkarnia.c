#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>

/* ===== SIGINT ===== */
void sigint_handler(int sig) {
    (void)sig;
    printf("\n[MAIN] Zatrzymanie symulacji...\n");
}

/* ===== Pamięć dzielona ===== */
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0666);
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

/* ===== Semafory ===== */
int init_semaphores() {
    int semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1); // mutex
    semctl(semid, 1, SETVAL, 0); // liczba ciastek
    return semid;
}

void sem_down(int semid, int semnum) {
    struct sembuf op = { semnum, -1, 0 };
    semop(semid, &op, 1);
}

void sem_up(int semid, int semnum) {
    struct sembuf op = { semnum, 1, 0 };
    semop(semid, &op, 1);
}

/* ===== Kolejka komunikatów ===== */
int init_msg_queue() {
    int msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return msgid;
}
