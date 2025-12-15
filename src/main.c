#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "ciastkarnia.h"

shared_data_t *shm;
int sem_magazyn;
int sem_klienci;

void proces_piekarz(int id);
void proces_kasjer(int id);
void proces_klient(int id, int limit);

int main() {
    int P = 2, K = 2, C = 10;
    int max_magazyn = 20;
    int limit_klientow = 5;

    int shm_id;
    init_shared_memory(&shm, &shm_id, max_magazyn);

    sem_magazyn = init_semaphore();
    sem_klienci = init_semaphore();

    signal(SIGINT, sigint_handler);

    pid_t pid;

    for (int i = 0; i < P; i++)
        if ((pid = fork()) == 0) proces_piekarz(i);

    for (int i = 0; i < K; i++)
        if ((pid = fork()) == 0) proces_kasjer(i);

    for (int i = 0; i < C; i++)
        if ((pid = fork()) == 0) proces_klient(i, limit_klientow);

    while (running) pause();

    printf("[MAIN] Zamykanie...\n");
    cleanup_shared_memory(shm_id, shm);
    return 0;
}
