#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm) {
    signal(SIGINT, SIG_DFL);

    while (1) {
        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }
        sleep(1);
    }
}

/* ===== KASJER ===== */
void proces_kasjer(int id) {
    signal(SIGINT, SIG_DFL);

    while (1) {
        printf("[Kasjer %d] Obsługuje klienta\n", id);
        sleep(2);
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id) {
    printf("[Klient %d] Wchodzi do sklepu\n", id);
    sleep(1);
    printf("[Klient %d] Wychodzi ze sklepu\n", id);
}
