#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#include "ciastkarnia.h"

int main() {
    int P = 2;
    int K = 2;
    int C = 5;
    int max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;

    init_shared_memory(&shm, &shm_id, max_magazyn);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("=== START CIASTKARNI ===\n");
    printf("CTRL+C aby zakończyć\n");

    pid_t dzieci[128];
    int dcount = 0;

    for (int i = 0; i < P; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_piekarz(i, shm);
        dzieci[dcount++] = pid;
    }

    for (int i = 0; i < K; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_kasjer(i);
        dzieci[dcount++] = pid;
    }

    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_klient(i);
        dzieci[dcount++] = pid;
    }

    while (running) pause();

    printf("\n[MAIN] Zamykanie procesów...\n");

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);

    printf("[MAIN] KONIEC SYMULACJI\n");
    return 0;
}
