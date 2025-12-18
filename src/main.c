#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


int main() {
    int P = 2;
    int K = 2;
    int C = 5;
    int max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;

    init_shared_memory(&shm, &shm_id, max_magazyn);

    // Prosta obsługa SIGINT
    signal(SIGINT, sigint_handler);

    pid_t dzieci[64];
    int dcount = 0;

    printf("=== START SYMULACJI CIASTKARNI ===\n");

    for (int i = 0; i < P; i++) {
        if ((dzieci[dcount++] = fork()) == 0)
            proces_piekarz(i, shm);
    }

    for (int i = 0; i < K; i++) {
        if ((dzieci[dcount++] = fork()) == 0)
            proces_kasjer(i);
    }

    for (int i = 0; i < C; i++) {
        if ((dzieci[dcount++] = fork()) == 0)
            proces_klient(i);
    }

    // Główna pętla – czekamy na CTRL+C
    while (running)
        sleep(1);

    printf("\n[MAIN] Zamykanie procesów...\n");

    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);

    printf("=== KONIEC SYMULACJI ===\n");
    return 0;
}
