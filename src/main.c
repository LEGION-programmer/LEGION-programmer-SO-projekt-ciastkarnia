#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

void sigint_main(int sig) {
    (void)sig;
    running = 0;
}

int main() {
    printf("[System] Otwieramy ciastkarnię...\n");
    signal(SIGINT, sigint_main);
    shared_data_t *shm = init_shared_memory();

    // 1. Uruchomienie piekarzy
    pid_t piekarze[2];
    for (int i = 0; i < 2; i++) {
        if ((piekarze[i] = fork()) == 0) {
            srand(time(NULL) ^ (getpid() << 1));
            proces_piekarz(i, shm);
            exit(0);
        }
    }

    // 2. Uruchomienie kasjera
    pid_t kasjer = fork();
    if (kasjer == 0) {
        proces_kasjer(0, shm);
        exit(0);
    }

    // 3. Generowanie klientów w odstępach czasu
    srand(time(NULL));
    for (int i = 0; i < MAX_KLIENCI && running; i++) {
        int ile = (rand() % 3) + 1;
        if (fork() == 0) {
            srand(time(NULL) ^ (getpid() << 1));
            proces_klient(i, shm, ile);
            exit(0);
        }
        // Nowy klient pojawia się co 1-2 sekundy
        usleep((rand() % 1000 + 1000) * 1000); 
    }

    // Czekamy, aż wszyscy klienci z kolejki zostaną obsłużeni
    while (shm->klienci_obsluzeni < shm->klienci_przyszli) {
        usleep(500000);
    }

    printf("\n[System] Brak nowych klientów. Zamykamy lokal...\n");
    running = 0; // Zatrzymuje pętle piekarzy i kasjera

    // Sprzątanie procesów
    for (int i = 0; i < 2; i++) waitpid(piekarze[i], NULL, 0);
    waitpid(kasjer, NULL, 0);
    while (wait(NULL) > 0);

    print_stats(shm);
    cleanup_shared_memory(shm);

    return 0;
}