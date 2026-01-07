#include "ciastkarnia.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

void init_shared_memory(shared_data_t **shm, int *shmid, int max) {
    *shmid = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0666);
    *shm = shmat(*shmid, NULL, 0);

    (*shm)->ciastka = 0;
    (*shm)->max_ciasta = max;
    (*shm)->kolejka = 0;

    (*shm)->klienci_przyszli = 0;
    (*shm)->klienci_obsluzeni = 0;
    (*shm)->sprzedane_ciasta = 0;
}

void cleanup_shared_memory(int shmid, shared_data_t *shm) {
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
}

int init_semaphore() {
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
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

int init_queue() {
    return msgget(IPC_PRIVATE, IPC_CREAT | 0666);
}
