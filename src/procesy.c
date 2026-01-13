#define _DEFAULT_SOURCE
#include "ciastkarnia.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

extern volatile sig_atomic_t running;

void proces_piekarz(int id, shared_data_t *shm) {
    while (running) {
        // Pieczenie trwa od 1 do 2 sekund
        usleep((rand() % 1000 + 1000) * 1000); 
        
        pthread_mutex_lock(&shm->mutex);
        if (shm->magazyn < MAX_MAGAZYN) {
            shm->magazyn++;
            printf("[Piekarz %d] Upiekł ciastko. Stan: %d/%d\n", id, shm->magazyn, MAX_MAGAZYN);
            fflush(stdout);
        }
        pthread_mutex_unlock(&shm->mutex);
    }
    printf("[Piekarz %d] Kończy zmianę i wychodzi.\n", id);
}

void proces_kasjer(int id, shared_data_t *shm) {
    (void)id; 
    while (running || shm->klienci_obsluzeni < shm->klienci_przyszli) {
        // Kasjer po prostu monitoruje sklep co pół sekundy
        usleep(500000);
    }
    printf("[System] Kasjer zamknął kasę.\n");
}

void proces_klient(int id, shared_data_t *shm, int ile) {
    pthread_mutex_lock(&shm->mutex);
    shm->klienci_przyszli++;
    pthread_mutex_unlock(&shm->mutex);

    printf("--> [Klient %2d] Wchodzi, chce kupić: %d\n", id, ile);
    fflush(stdout);

    int kupione = 0;
    while (running && !kupione) {
        pthread_mutex_lock(&shm->mutex);
        if (shm->magazyn >= ile) {
            shm->magazyn -= ile;
            shm->klienci_obsluzeni++;
            shm->sprzedane_ciastka += ile;
            printf("<-- [Kasjer] Sprzedano %d ciastek klientowi %d. Zostało: %d\n", ile, id, shm->magazyn);
            fflush(stdout);
            kupione = 1;
        }
        pthread_mutex_unlock(&shm->mutex);
        
        if (!kupione) {
            // Jeśli nie ma towaru, klient czeka cierpliwie 1.5 sekundy
            usleep(1500000); 
        }
    }
}