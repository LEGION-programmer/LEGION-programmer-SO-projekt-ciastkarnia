#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h> // usleep

void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0660);
    if (*shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    *shm = (shared_data_t *)shmat(*shm_id, NULL, 0);
    if (*shm == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    (*shm)->ciastka = 0;
    (*shm)->max = max_magazyn;
    (*shm)->klienci = 0;
}

void cleanup_shared_memory(int shm_id, shared_data_t *shm) {
    if (shmdt(shm) == -1) perror("shmdt");
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) perror("shmctl");
}
