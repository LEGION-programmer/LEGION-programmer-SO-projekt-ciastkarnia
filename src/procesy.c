#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
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
    exit(0);
}

void proces_kasjer(int id) {
    while (running) {
        printf("[Kasjer %d] Czeka na klienta...\n", id);
        sleep(2);
    }
    exit(0);
}

void proces_klient(int id) {
    printf("[Klient %d] Wchodzi do sklepu\n", id);
    sleep(1 + rand() % 3);
    printf("[Klient %d] Wychodzi\n", id);
    exit(0);
}

void generator_klientow(void) {
    int id = 0;
    srand(getpid());

    while (running) {
        pid_t pid = fork();
        if (pid == 0) {
            proces_klient(id++);
        }
        sleep(1 + rand() % 2);
    }
    exit(0);
}
