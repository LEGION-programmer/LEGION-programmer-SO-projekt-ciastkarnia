#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>

void proces_piekarz(int id, shared_data_t *shm, int semid) {
    while (running) {
        semaphore_lock(semid);
        if (shm->ciastka < shm->max) {
            shm->ciastka++;
            printf("[Piekarz %d] Ciastko (%d/%d)\n",
                   id, shm->ciastka, shm->max);
        }
        semaphore_unlock(semid);
        sleep(1);
    }
    exit(0);
}

void proces_klient(int id, int msgid) {
    client_msg_t msg;
    msg.mtype = 1;
    msg.client_id = id;

    printf("[Klient %d] Ustawia się w kolejce\n", id);
    msgsnd(msgid, &msg, sizeof(int), 0);

    exit(0);
}

void proces_kasjer(int id, int msgid) {
    client_msg_t msg;

    while (running) {
        if (msgrcv(msgid, &msg, sizeof(int), 1, IPC_NOWAIT) > 0) {
            printf("[Kasjer %d] Obsługuje klienta %d\n",
                   id, msg.client_id);
            sleep(2);
        } else {
            sleep(1);
        }
    }
    exit(0);
}
