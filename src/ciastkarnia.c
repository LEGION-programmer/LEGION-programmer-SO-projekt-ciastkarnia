#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    (void)sig;
    running = 0;
}

// ===== Shared Memory =====
void init_shared_memory(shared_data_t **shm, int *shm_id, int max_magazyn) {
    *shm_id = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | 0660);
    *shm = shmat(*shm_id, NULL, 0);
    (*shm)->ciastka = 0;
    (*shm)->max = max_magazyn;
}

void cleanup_shared_memory(int shm_id, shared_data_t *shm) {
    shmdt(shm);
    shmctl(shm_id, IPC_RMID, NULL);
}

// ===== Semaphore =====
int init_semaphore(void) {
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0660);
    semctl(semid, 0, SETVAL, 1);
    return semid;
}

void semaphore_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void semaphore_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void remove_semaphore(int semid) {
    semctl(semid, 0, IPC_RMID);
}

// ===== Message Queue =====
int init_queue(void) {
    return msgget(IPC_PRIVATE, IPC_CREAT | 0660);
}

void remove_queue(int msgid) {
    msgctl(msgid, IPC_RMID, NULL);
}
