#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    signal(SIGINT, SIG_DFL);

    while (1) {
        sem_down(semid, 0);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
            sem_up(semid, 1);
        }

        sem_up(semid, 0);
        sleep(1);
    }
}

/* ===== KASJER ===== */
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid) {
    signal(SIGINT, SIG_DFL);

    klient_msg_t msg;

    while (1) {
        msgrcv(msgid, &msg, sizeof(int), 1, 0);

        sem_down(semid, 1);
        sem_down(semid, 0);

        shm->ciastka--;
        printf("[Kasjer %d] Obsłużył klienta %d (%d/%d)\n",
               id, msg.klient_id, shm->ciastka, shm->max);

        sem_up(semid, 0);
        sleep(1);
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id, int msgid) {
    klient_msg_t msg;
    msg.mtype = 1;
    msg.klient_id = id;

    printf("[Klient %d] Wchodzi i ustawia się w kolejce\n", id);
    msgsnd(msgid, &msg, sizeof(int), 0);

    sleep(2);
    printf("[Klient %d] Został obsłużony i wychodzi\n", id);
}
