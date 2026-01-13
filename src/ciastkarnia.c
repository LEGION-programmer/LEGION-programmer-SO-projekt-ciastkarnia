#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

shared_data_t* init_shared_memory() {
    // Rezerwacja pamięci współdzielonej dostępnej dla wszystkich procesów potomnych
    shared_data_t *shm = mmap(NULL, sizeof(shared_data_t), 
                              PROT_READ | PROT_WRITE, 
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    shm->magazyn = 0;
    shm->klienci_przyszli = 0;
    shm->klienci_obsluzeni = 0;
    shm->sprzedane_ciastka = 0;

    // Konfiguracja muteksu do pracy między procesami (PTHREAD_PROCESS_SHARED)
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    return shm;
}

void cleanup_shared_memory(shared_data_t *shm) {
    pthread_mutex_destroy(&shm->mutex);
    munmap(shm, sizeof(shared_data_t));
}

void print_stats(shared_data_t *shm) {
    printf("\n====================================\n");
    printf("   PODSUMOWANIE DNIA W CIASTKARNI\n");
    printf("====================================\n");
    printf(" Sprzedane ciastka:    %d\n", shm->sprzedane_ciastka);
    printf(" Klienci obsłużeni:    %d z %d\n", shm->klienci_obsluzeni, shm->klienci_przyszli);
    printf(" Ciastka w magazynie:  %d\n", shm->magazyn);
    printf("====================================\n");
}