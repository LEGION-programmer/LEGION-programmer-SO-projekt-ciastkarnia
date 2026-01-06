#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running) {
        sem_down(semid);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            shm->wyprodukowane++;
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
        if (msgrcv(msgid, &msg, sizeof(client_msg_t) - sizeof(long), 1, IPC_NOWAIT) > 0) {
            sem_down(semid);

            int ile = msg.ile_chce;
            if (shm->ciastka >= ile) {
                shm->ciastka -= ile;
                shm->sprzedane += ile;
                shm->obsluzeni_klienci++;

                printf("[Kasjer %d] klient %d kupił %d (zostało %d)\n",
                       id, msg.client_id, ile, shm->ciastka);
            } else {
                /* brak ciastek – klient wraca do kolejki */
                msgsnd(msgid, &msg, sizeof(client_msg_t) - sizeof(long), 0);
            }

            sem_up(semid);
        } else {
            usleep(100000);
        }
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id, shared_data_t *shm, int semid, int msgid) {
    sem_down(semid);
    if (shm->klienci_w_sklepie >= shm->max_klientow) {
        sem_up(semid);
        exit(0);
    }
    shm->klienci_w_sklepie++;
    sem_up(semid);

    client_msg_t msg;
    msg.mtype = 1;
    msg.client_id = id;
    msg.ile_chce = (rand() % 3) + 1;

    printf("[Klient %d] chce %d ciastek\n", id, msg.ile_chce);
    msgsnd(msgid, &msg, sizeof(client_msg_t) - sizeof(long), 0);

    sleep(1);

    sem_down(semid);
    shm->klienci_w_sklepie--;
    sem_up(semid);

    exit(0);
}
