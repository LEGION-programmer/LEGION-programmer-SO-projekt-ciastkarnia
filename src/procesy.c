#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>

/* ===== PIEKARZ ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running) {
        sem_down(semid);

        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] wypiekł ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }

        sem_up(semid);
        sleep(1);
    }

    printf("[Piekarz %d] kończy pracę\n", id);
    _exit(0);
}

/* ===== KASJER ===== */
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid) {
    client_msg_t msg;

    while (running) {
        ssize_t r = msgrcv(
            msgid,
            &msg,
            sizeof(int),
            1,
            IPC_NOWAIT
        );

        if (r == -1) {
            if (errno == ENOMSG) {
                usleep(200000); // brak klientów
                continue;
            }
            perror("msgrcv");
            break;
        }

        printf("[Kasjer %d] obsługuje klienta %d\n", id, msg.client_id);
        sleep(2); // czas obsługi

        sem_down(semid);
        if (shm->ciastka > 0) {
            shm->ciastka--;
            printf("[Kasjer %d] sprzedał ciastko klientowi %d (zostało %d)\n",
                   id, msg.client_id, shm->ciastka);
        } else {
            printf("[Kasjer %d] brak ciastek dla klienta %d\n",
                   id, msg.client_id);
        }
        sem_up(semid);
    }

    printf("[Kasjer %d] zamyka kasę\n", id);
    _exit(0);
}

/* ===== KLIENT ===== */
void proces_klient(int id, int msgid) {
    client_msg_t msg;
    msg.mtype = 1;
    msg.client_id = id;

    printf("[Klient %d] wchodzi do sklepu\n", id);
    sleep(1);

    printf("[Klient %d] ustawia się w kolejce\n", id);
    msgsnd(msgid, &msg, sizeof(int), 0);

    sleep(2); // symulacja pobytu
    printf("[Klient %d] wychodzi ze sklepu\n", id);

    _exit(0);
}
