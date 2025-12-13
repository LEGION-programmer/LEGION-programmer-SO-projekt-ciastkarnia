#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>

static void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

static void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

/* ===== PIEKARZ ===== */

void proces_piekarz(int id, shared_data_t *shm, int semid) {
    signal(SIGINT, sigint_handler);

    while (running) {
        sem_lock(semid);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        sem_unlock(semid);
        sleep(1);
    }

    printf("[Piekarz %d] Kończy pracę\n", id);
}

/* ===== KASJER ===== */

void proces_kasjer(int id, shared_data_t *shm, int semid) {
    signal(SIGINT, sigint_handler);

    while (running) {
        sem_lock(semid);

        if (shm->ciastka > 0) {
            shm->ciastka--;
            printf("[Kasjer %d] Sprzedał ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        sem_unlock(semid);
        sleep(2);
    }

    printf("[Kasjer %d] Zamyka kasę\n", id);
}

/* ===== KLIENT ===== */

void proces_klient(int id, shared_data_t *shm, int semid) {
    signal(SIGINT, sigint_handler);

    sem_lock(semid);

    if (shm->ciastka > 0) {
        shm->ciastka--;
        printf("[Klient %d] Kupił ciastko (%d/%d)\n",
               id, shm->ciastka, shm->max);
    } else {
        printf("[Klient %d] Brak ciastek – wychodzi\n", id);
    }

    sem_unlock(semid);
}
