#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running && shm->klienci_pozostali > 0) {
        sem_down(semid, 0);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] +1 (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        sem_up(semid, 0);
        sleep(1);
    }
    exit(0);
}

/* ===== KASJER ===== */
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid) {
    client_msg_t msg;

    while (running && shm->klienci_pozostali > 0) {
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0) < 0)
            continue;

        int done = 0;

        while (!done && running) {
            sem_down(semid, 0);

            if (shm->ciastka >= msg.ile_chce) {
                shm->ciastka -= msg.ile_chce;
                shm->klienci_pozostali--;

                printf("[Kasjer %d] klient %d kupił %d (zostało %d)\n",
                       id, msg.client_id, msg.ile_chce, shm->ciastka);

                done = 1;
            }

            sem_up(semid, 0);

            if (!done)
                sleep(1);
        }

        sem_up(semid, 1); // klient wychodzi ze sklepu
    }
    exit(0);
}

/* ===== KLIENT ===== */
void proces_klient(int id, int semid, int msgid) {
    srand(getpid());

    sem_down(semid, 1); // wejście do sklepu

    client_msg_t msg;
    msg.mtype = 1;
    msg.client_id = id;
    msg.ile_chce = (rand() % 3) + 1;

    printf("[Klient %d] wszedł do sklepu (chce %d)\n",
           id, msg.ile_chce);

    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
    exit(0);
}
