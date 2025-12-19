#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running) {
        semaphore_lock(semid);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        semaphore_unlock(semid);
        sleep(1);
    }
    exit(0);
}

void proces_kasjer(int id, shared_data_t *shm, int semid) {
    while (running) {
        semaphore_lock(semid);
        printf("[Kasjer %d] Widzi %d ciastek\n", id, shm->ciastka);
        semaphore_unlock(semid);
        sleep(2);
    }
    exit(0);
}

void proces_klient(int id) {
    printf("[Klient %d] Wszedł do sklepu\n", id);
    sleep(1);
    printf("[Klient %d] Wyszedł\n", id);
    exit(0);
}
