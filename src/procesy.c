#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// =======================
// Proces piekarza
// =======================
void proces_piekarz(int id, shared_data_t *shm) {
    while(1) {
        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] wypiekł ciastko. Stan magazynu: %d/%d\n",
                   id, shm->ciastka, shm->max);
        }
        sleep(1); // zamiast usleep(200000)
    }
}

// =======================
// Proces kasjera
// =======================
void proces_kasjer(int id) {
    while(1) {
        printf("[Kasjer %d] obsługuje klienta...\n", id);
        sleep(1); // zamiast usleep(300000)
    }
}

// =======================
// Proces klienta
// =======================
void proces_klient(int id) {
    printf("[Klient %d] wchodzi do sklepu.\n", id);
    sleep(1); // zamiast usleep(100000)
    printf("[Klient %d] wychodzi ze sklepu.\n", id);
}
