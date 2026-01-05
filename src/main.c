#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>

int main() {
    int P = 2;
    int K = 2;
    int C = 10;
    int MAX_W_SKLEPIE = 3;
    int max_magazyn = 10;

    shared_data_t *shm;
    int shm_id, semid, msgid;

    init_shared_memory(&shm, &shm_id, max_magazyn, C);
    semid = init_semaphores(MAX_W_SKLEPIE);
    msgid = init_queue();

    struct sigaction sa = {0};
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("=== CIASTKARNIA START (limit klientów: %d) ===\n", MAX_W_SKLEPIE);

    for (int i = 0; i < P; i++)
        if (fork() == 0)
            proces_piekarz(i, shm, semid);

    for (int i = 0; i < K; i++)
        if (fork() == 0)
            proces_kasjer(i, shm, semid, msgid);

    for (int i = 0; i < C; i++) {
        sleep(1);
        if (fork() == 0)
            proces_klient(i, semid, msgid);
    }

    while (shm->klienci_pozostali > 0)
        sleep(1);

    printf("\n[MAIN] Koniec symulacji\n");
    kill(0, SIGTERM);

    while (wait(NULL) > 0);

    cleanup_shared_memory(shm_id, shm);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
