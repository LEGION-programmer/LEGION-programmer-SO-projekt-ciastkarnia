#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define MAG_KEY 1234
#define SEM_KEY 5678

typedef struct {
    int pojemnosc;
    int stan;
} Magazyn;

int shm_id;
int sem_id;
Magazyn *mag;

void init_magazyn(int pojemnosc) {
    shm_id = shmget(MAG_KEY, sizeof(Magazyn), IPC_CREAT | 0660);
    if(shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    mag = (Magazyn*) shmat(shm_id, NULL, 0);
    if(mag == (void*) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    mag->pojemnosc = pojemnosc;
    mag->stan = 0;

    sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0660);
    if(sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    if(semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

void piekarz_proc(int id) {
    while(1) {
        // Prosty semafor: P (lock)
        struct sembuf op = {0, -1, 0};
        semop(sem_id, &op, 1);

        if(mag->stan < mag->pojemnosc) {
            mag->stan++;
            printf("Piekarz %d wypiekł ciastko. Stan magazynu: %d/%d\n",
                   id, mag->stan, mag->pojemnosc);
        }

        // V (unlock)
        op.sem_op = 1;
        semop(sem_id, &op, 1);

        usleep(500000); // 0.5s
    }
}

void cleanup_magazyn() {
    if(shmdt(mag) == -1) {
        perror("shmdt");
    }
    if(shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }
    if(semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
    }
}
