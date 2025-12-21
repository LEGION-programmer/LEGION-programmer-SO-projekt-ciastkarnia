#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
    signal(SIGINT, sigint_handler);

    int P = 2, K = 2, C = 5;
    int max_magazyn = 10;

    shared_data_t *shm;
    int shm_id, semid, msgid;

    init_shared_memory(&shm, &shm_id, max_magazyn);
    semid = init_semaphore();
    msgid = init_queue();

    pid_t dzieci[64];
    int dcount = 0;

    for (int i = 0; i < P; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_piekarz(i, shm, semid);

    for (int i = 0; i < K; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_kasjer(i, msgid);

    for (int i = 0; i < C; i++)
        if ((dzieci[dcount++] = fork()) == 0)
            proces_klient(i, msgid);

    while (running)
        pause();

    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);
    remove_semaphore(semid);
    remove_queue(msgid);

    printf("=== KONIEC SYMULACJI ===\n");
    return 0;
}
