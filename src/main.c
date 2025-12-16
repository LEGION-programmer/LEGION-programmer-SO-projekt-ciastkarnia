#define _POSIX_C_SOURCE 200809L

#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

int main() {
    int P = 2, K = 2, C = 5;
    int max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;

    init_shared_memory(&shm, &shm_id, max_magazyn);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    pid_t dzieci[64];
    int dcount = 0;

    for (int i = 0; i < P; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_piekarz(i, shm), exit(0);

    for (int i = 0; i < K; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_kasjer(i), exit(0);

    for (int i = 0; i < C; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_klient(i), exit(0);

    printf("=== Symulacja działa (CTRL+C aby zakończyć) ===\n");

    while (running) pause();

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);
    return 0;
}
