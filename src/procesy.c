#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
}

void proces_kasjer(int id) {
    while (running) {
        printf("[Kasjer %d] czeka na klienta...\n", id);
        sleep(2);
    }
    printf("[Kasjer %d] kończy pracę\n", id);
}

void proces_klient(int id) {
    if (!running) exit(0);

    printf("[Klient %d] wchodzi do sklepu\n", id);
    sleep(1);
    printf("[Klient %d] wychodzi ze sklepu\n", id);
}
