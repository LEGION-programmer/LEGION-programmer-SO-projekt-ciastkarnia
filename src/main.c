#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int P = 2;
    int K = 2;
    int C = 5;
    int max_magazyn = 20;

    signal(SIGINT, sigint_handler);

    shared_data_t *shm;
    int shm_id = 0;
    int semid = 0;

    init_shared_memory(&shm, &shm_id, max_magazyn);
    semid = init_semaphore();

    pid_t dzieci[64];
    int dcount = 0;

    printf("=== START CIASTKARNI ===\n");

    for (int i = 0; i < P; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_piekarz(i, shm, semid), exit(0);

    for (int i = 0; i < K; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_kasjer(i, shm, semid), exit(0);

    for (int i = 0; i < C; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_klient(i, shm, semid), exit(0);

    while (running)
        pause();

    printf("\n[MAIN] Zamykanie ciastkarni...\n");

    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_semaphore(semid);
    cleanup_shared_memory(shm_id, shm);

    printf("=== KONIEC ===\n");
    return 0;
}
