#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    signal(SIGINT, SIG_DFL);

    while (1) {
        sem_down(semid, 0); // mutex

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
            sem_up(semid, 1); // nowe ciastko
        }

        sem_up(semid, 0); // mutex
        sleep(1);
    }
}

/* ===== KASJER ===== */
void proces_kasjer(int id, shared_data_t *shm, int semid) {
    signal(SIGINT, SIG_DFL);

    while (1) {
        sem_down(semid, 1); // czekaj na ciastko
        sem_down(semid, 0);

        shm->ciastka--;
        printf("[Kasjer %d] Sprzedał ciastko (%d/%d)\n",
               id, shm->ciastka, shm->max);

        sem_up(semid, 0);
        sleep(2);
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id, shared_data_t *shm, int semid) {
    (void)shm;
    (void)semid;

    printf("[Klient %d] Wchodzi do sklepu\n", id);
    sleep(1);
    printf("[Klient %d] Ustawia się w kolejce\n", id);
    sleep(2);
    printf("[Klient %d] Wychodzi ze sklepu\n", id);
}
