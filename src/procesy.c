#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>

void proces_piekarz(int id, shared_data_t *shm) {
    while (running) {
        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Ciastko %d/%d\n",
                   id, shm->ciastka, shm->max);
        }
        sleep(1);
    }
    printf("[Piekarz %d] Kończy pracę\n", id);
}

void proces_kasjer(int id) {
    while (running) {
        printf("[Kasjer %d] Czeka na klienta...\n", id);
        sleep(2);
    }
    printf("[Kasjer %d] Kończy pracę\n", id);
}

void proces_klient(int id) {
    printf("[Klient %d] Wchodzi do sklepu\n", id);
    sleep(1);
    printf("[Klient %d] Wychodzi\n", id);
}
