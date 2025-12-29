#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "ciastkarnia.h"

int main() {
    int P = 2, K = 2, C = 6;
    int max_magazyn = 10;

    shared_data_t *shm;
    int shm_id = 0;
    int semid = 0;

    init_shared_memory(&shm, &shm_id, max_magazyn);
    semid = init_semaphore();

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("=== START CIASTKARNI === (CTRL+C kończy)\n");

    pid_t pid;

    for (int i = 0; i < P; i++)
        if ((pid = fork()) == 0)
            proces_piekarz(i, shm, semid);

    for (int i = 0; i < K; i++)
        if ((pid = fork()) == 0)
            proces_kasjer(i);

    for (int i = 0; i < C; i++) {
        sleep(1);
        if ((pid = fork()) == 0)
            proces_klient(i, shm, semid);
    }

    while (running)
        pause();

    printf("\n[MAIN] Zamykanie symulacji...\n");

    while (wait(NULL) > 0);
    cleanup_shared_memory(shm_id, shm);

    return 0;
}
