#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/msg.h>

#include "ciastkarnia.h"

int main() {
    int P = 2, K = 2, C = 8;
    int max_magazyn = 10;

    shared_data_t *shm;
    int shm_id, semid, msgid;

    init_shared_memory(&shm, &shm_id, max_magazyn);
    semid = init_semaphore();
    msgid = init_queue();

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("=== CIASTKARNIA START (CTRL+C) ===\n");

    for (int i = 0; i < P; i++)
        if (fork() == 0)
            proces_piekarz(i, shm, semid);

    for (int i = 0; i < K; i++)
        if (fork() == 0)
            proces_kasjer(i, shm, semid, msgid);

    for (int i = 0; i < C; i++) {
        sleep(1);
        if (fork() == 0)
            proces_klient(i, msgid);
    }

    while (running)
        pause();

    printf("\n[MAIN] Zamykanie symulacji...\n");

    while (wait(NULL) > 0);

    cleanup_shared_memory(shm_id, shm);
    cleanup_semaphore(semid);
    msgctl(msgid, IPC_RMID, NULL);

    printf("=== CIASTKARNIA ZAMKNIĘTA ===\n");
    return 0;
}
