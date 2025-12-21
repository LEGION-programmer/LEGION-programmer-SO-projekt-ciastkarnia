#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main() {
    signal(SIGINT, sigint_handler);

    int P = 2;
    int K = 2;
    int max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;
    init_shared_memory(&shm, &shm_id, max_magazyn);

    pid_t dzieci[32];
    int dcount = 0;

    for (int i = 0; i < P; i++) {
        if ((dzieci[dcount] = fork()) == 0)
            proces_piekarz(i, shm);
        dcount++;
    }

    for (int i = 0; i < K; i++) {
        if ((dzieci[dcount] = fork()) == 0)
            proces_kasjer(i);
        dcount++;
    }

    if ((dzieci[dcount] = fork()) == 0)
        generator_klientow();
    dcount++;

    while (running)
        pause();

    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);
    printf("=== KONIEC SYMULACJI ===\n");
    return 0;
}
