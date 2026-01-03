#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running) {
        sem_down(semid);
        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] +1 (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }
        sem_up(semid);
        sleep(1);
    }
}

/* ===== KASJER ===== */
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid) {
    client_msg_t msg;

    while (running) {
        if (msgrcv(msgid, &msg, sizeof(int), 1, 0) > 0) {
            sem_down(semid);

            if (shm->ciastka > 0) {
                shm->ciastka--;
                printf("[Kasjer %d] obsłużył klienta %d (pozostało %d)\n",
                       id, msg.client_id, shm->ciastka);
            } else {
                printf("[Kasjer %d] brak ciastek dla klienta %d\n",
                       id, msg.client_id);
            }

            sem_up(semid);
            sleep(2);
        }
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id, int msgid) {
    client_msg_t msg;
    msg.mtype = 1;
    msg.client_id = id;

    printf("[Klient %d] ustawia się w kolejce\n", id);
    msgsnd(msgid, &msg, sizeof(int), 0);
}
