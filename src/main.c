#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include "ciastkarnia.h"

int main() {
    int P = 2, K = 2, C = 12;
    int max_magazyn = 10;
    int max_klientow = 4;

    shared_data_t *shm;
    int shm_id, semid, msgid;

    init_shared_memory(&shm, &shm_id, max_magazyn, max_klientow);
    semid = init_semaphore();
    msgid = init_queue();

    struct sigaction sa = {0};
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("=== START CIASTKARNI (CTRL+C) ===\n");

    for (int i = 0; i < P; i++)
        if (fork() == 0)
            proces_piekarz(i, shm, semid);

    for (int i = 0; i < K; i++)
        if (fork() == 0)
            proces_kasjer(i, shm, semid, msgid);

    for (int i = 0; i < C; i++) {
        sleep(1);
        if (fork() == 0)
            proces_klient(i, shm, semid, msgid);
    }

    while (running)
        pause();

    while (wait(NULL) > 0);

    printf("\n=== RAPORT KOŃCOWY ===\n");
    printf("Wyprodukowano: %d\n", shm->wyprodukowane);
    printf("Sprzedano:     %d\n", shm->sprzedane);
    printf("Obsłużono:     %d klientów\n", shm->obsluzeni_klienci);
    printf("Pozostało:     %d\n", shm->ciastka);

    cleanup_shared_memory(shm_id, shm);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
