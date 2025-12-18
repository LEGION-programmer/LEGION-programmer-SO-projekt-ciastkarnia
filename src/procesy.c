#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// =======================
// Piekarz
// =======================
void proces_piekarz(int id, shared_data_t *shm) {
    while (running) {
        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }
        sleep(1);
    }
    printf("[Piekarz %d] kończy pracę\n", id);
    exit(0);
}

// =======================
// Kasjer
// =======================
void proces_kasjer(int id) {
    while (running) {
        printf("[Kasjer %d] czeka na klienta\n", id);
        sleep(2);
    }
    printf("[Kasjer %d] kończy pracę\n", id);
    exit(0);
}

// =======================
// Klient
// =======================
void proces_klient(int id) {
    printf("[Klient %d] wchodzi do sklepu\n", id);
    sleep(2);
    printf("[Klient %d] wychodzi ze sklepu\n", id);
    exit(0);
}
