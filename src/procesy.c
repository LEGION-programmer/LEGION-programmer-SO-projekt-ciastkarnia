#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running) {
        sem_down(semid);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] +1 ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        sem_up(semid);
        sleep(1);
    }
}

/* ===== KASJER ===== */
void proces_kasjer(int id) {
    while (running) {
        printf("[Kasjer %d] czeka na klienta...\n", id);
        sleep(2);
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id, shared_data_t *shm, int semid) {
    printf("[Klient %d] wchodzi do sklepu\n", id);

    sem_down(semid);

    if (shm->ciastka > 0) {
        shm->ciastka--;
        printf("[Klient %d] kupił ciastko (pozostało %d)\n",
               id, shm->ciastka);
    } else {
        printf("[Klient %d] brak ciastek\n", id);
    }

    sem_up(semid);

    printf("[Klient %d] wychodzi\n", id);
}
