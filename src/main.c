#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "ciastkarnia.h"

int main() {
    int P = 2, K = 2, C = 6, max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;

    signal(SIGINT, sigint_handler);

    init_shared_memory(&shm, &shm_id, max_magazyn);
    int semid = init_semaphores();
    int msgid = init_msg_queue();

    printf("=== START CIASTKARNI ===\n");

    pid_t dzieci[64];
    int dcount = 0;

    for (int i = 0; i < P; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_piekarz(i, shm, semid), exit(0);
        dzieci[dcount++] = pid;
    }

    for (int i = 0; i < K; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_kasjer(i, shm, semid, msgid), exit(0);
        dzieci[dcount++] = pid;
    }

    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid == 0) proces_klient(i, msgid), exit(0);
        dzieci[dcount++] = pid;
    }

    pause();

    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);

    printf("=== KONIEC ===\n");
    return 0;
}
