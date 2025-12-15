#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

extern int sem_magazyn;
extern int sem_klienci;
extern shared_data_t *shm;

// ===== PIEKARZ =====
void proces_piekarz(int id) {
    while (running) {
        sem_lock(sem_magazyn);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        sem_unlock(sem_magazyn);
        sleep(1);
    }
    exit(0);
}

// ===== KASJER =====
void proces_kasjer(int id) {
    while (running) {
        printf("[Kasjer %d] Czeka na klientów...\n", id);
        sleep(2);
    }
    exit(0);
}

// ===== KLIENT =====
void proces_klient(int id, int limit) {
    while (running) {
        sem_lock(sem_klienci);

        if (shm->klienci < limit) {
            shm->klienci++;
            printf("[Klient %d] Wszedł do sklepu (%d/%d)\n",
                   id, shm->klienci, limit);
            sem_unlock(sem_klienci);
            break;
        }

        sem_unlock(sem_klienci);
        sleep(1);
    }

    sleep(2); // zakupy

    sem_lock(sem_klienci);
    shm->klienci--;
    printf("[Klient %d] Wyszedł ze sklepu (%d/%d)\n",
           id, shm->klienci, limit);
    sem_unlock(sem_klienci);

    exit(0);
}
