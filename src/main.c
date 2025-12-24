#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "ciastkarnia.h"

int main() {
    int P = 2;   // piekarze
    int K = 2;   // kasjerzy
    int C = 5;   // klienci
    int max_magazyn = 20;

    shared_data_t *shm;
    int shm_id;

    signal(SIGINT, sigint_handler);

    init_shared_memory(&shm, &shm_id, max_magazyn);

    printf("=== START SYMULACJI CIASTKARNI ===\n");

    pid_t dzieci[64];
    int dcount = 0;

    /* Piekarze */
    for (int i = 0; i < P; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            proces_piekarz(i, shm);
            exit(0);
        }
        dzieci[dcount++] = pid;
    }

    /* Kasjerzy */
    for (int i = 0; i < K; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            proces_kasjer(i);
            exit(0);
        }
        dzieci[dcount++] = pid;
    }

    /* Klienci */
    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            proces_klient(i);
            exit(0);
        }
        dzieci[dcount++] = pid;
    }

    printf("CTRL+C aby zakończyć symulację\n");
    pause();

    printf("[MAIN] Zatrzymywanie procesów...\n");
    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGINT);

    for (int i = 0; i < dcount; i++)
        waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);

    printf("=== KONIEC SYMULACJI ===\n");
    return 0;
}
