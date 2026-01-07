#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <time.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    signal(SIGTERM, SIG_DFL);

    while (1) {
        sem_down(semid);
        if (shm->ciastka < shm->max_ciasta) {
            shm->ciastka++;
            printf("[Piekarz %d] +1 (%d/%d)\n",
                   id, shm->ciastka, shm->max_ciasta);
        }
        sem_up(semid);
        sleep(1);
    }
}

/* ===== KASJER ===== */
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid) {
    signal(SIGTERM, SIG_DFL);

    client_msg_t msg;

    while (1) {
        /* kasjer 2 tylko gdy kolejka >= 3 */
        if (id == 1) {
            sem_down(semid);
            int aktywny = (shm->kolejka >= 3);
            sem_up(semid);
            if (!aktywny) {
                sleep(1);
                continue;
            }
        }

        if (msgrcv(msgid, &msg, sizeof(client_msg_t) - sizeof(long), 1, 0) < 0)
            continue;

        sem_down(semid);
        shm->kolejka--;

        while (shm->ciastka < msg.ile_chce) {
            sem_up(semid);
            sleep(1);
            sem_down(semid);
        }

        shm->ciastka -= msg.ile_chce;
        shm->sprzedane_ciasta += msg.ile_chce;
        shm->klienci_obsluzeni++;

        printf("[Kasjer %d] klient %d kupił %d (zostało %d)\n",
               id, msg.client_id, msg.ile_chce, shm->ciastka);

        sem_up(semid);
        sleep(1);
    }
}

/* ===== KLIENT ===== */
void proces_klient(int id, int msgid) {
    srand(getpid());

    client_msg_t msg;
    msg.mtype = 1;
    msg.client_id = id;
    msg.ile_chce = (rand() % 3) + 1;

    printf("[Klient %d] chce %d ciastek\n", id, msg.ile_chce);
    msgsnd(msgid, &msg, sizeof(client_msg_t) - sizeof(long), 0);

    _exit(0);
}
