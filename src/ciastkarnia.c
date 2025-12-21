#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    (void)sig;
    running = 0;
    printf("\n[MAIN] SIGINT – kończę symulację\n");
}

void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0660);
    if (*shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    *shm = shmat(*shm_id, NULL, 0);
    if (*shm == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    (*shm)->ciastka = 0;
    (*shm)->max = max_magazyn;
    (*shm)->klienci = 0;
}

void cleanup_shared_memory(int shm_id, shared_data_t *shm) {
    shmdt(shm);
    shmctl(shm_id, IPC_RMID, NULL);
}
