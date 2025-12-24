#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "ciastkarnia.h"

int main() {
    int P = 2;
    int K = 2;
    int C = 5;
    int max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;

    signal(SIGINT, sigint_handler);

    init_shared_memory(&shm, &shm_id, max_magazyn);
    int semid = init_semaphores();

    printf("=== START SYMULACJI CIASTKARNI ===\n");

    pid_t dzieci[64];
    int dcount = 0;

    for (int i = 0; i < P; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_piekarz(i, shm, semid), exit(0);
        dzieci[dcount++] = pid;
    }

    for (int i = 0; i < K; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_kasjer(i, shm, semid), exit(0);
        dzieci[dcount++] = pid;
    }

    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_klient(i, shm, semid), exit(0);
        dzieci[dcount++] = pid;
    }

    printf("CTRL+C aby zakończyć symulację\n");
    pause();

    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);

    printf("=== KONIEC SYMULACJI ===\n");
    return 0;
}
