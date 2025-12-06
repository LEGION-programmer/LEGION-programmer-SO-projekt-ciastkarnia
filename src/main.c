#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // usleep, pause, fork
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include <signal.h>     // struct sigaction, sigaction, kill
#include <string.h>     // memset
#include "ciastkarnia.h"


// Deklaracje funkcji procesów
void proces_piekarz(int id, shared_data_t *shm);
void proces_kasjer(int id);
void proces_klient(int id);

// Handler SIGINT
void sigint_handler(int sig) {
    (void)sig;
    printf("\n[MAIN] Odebrano SIGINT – kończenie symulacji…\n");
}

int main() {
    int P = 2, K = 2, C = 5, max_magazyn = 20;
    shared_data_t *shm;
    int shm_id;
    init_shared_memory(&shm, &shm_id, max_magazyn);

    printf("=== START SYMULACJI CIASTKARNI ===\n");

    pid_t dzieci[128];
    int dcount = 0;

    // Piekarze
    for (int i = 0; i < P; i++) {
        pid_t pid = fork();
        if (pid == 0) { proces_piekarz(i, shm); exit(0); }
        dzieci[dcount++] = pid;
    }

    // Kasjerzy
    for (int i = 0; i < K; i++) {
        pid_t pid = fork();
        if (pid == 0) { proces_kasjer(i); exit(0); }
        dzieci[dcount++] = pid;
    }

    // Klienci
    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid == 0) { proces_klient(i); exit(0); }
        dzieci[dcount++] = pid;
    }

    // Obsługa SIGINT
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("Wciśnij CTRL+C aby zakończyć...\n");
    pause(); // czekamy na SIGINT

    printf("[MAIN] Zatrzymywanie procesów...\n");
    for (int i = 0; i < dcount; i++) kill(dzieci[i], SIGINT);
    for (int i = 0; i < dcount; i++) waitpid(dzieci[i], NULL, 0);

    cleanup_shared_memory(shm_id, shm);

    printf("=== KONIEC SYMULACJI ===\n");
    return 0;
}
