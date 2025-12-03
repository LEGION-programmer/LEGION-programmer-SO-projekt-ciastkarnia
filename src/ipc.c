#include "ipc.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int init_shared_memory(int *shmid, shm_data_t **shm_ptr, int capacity) {
    key_t key = ftok(".", 65);
    *shmid = shmget(key, sizeof(shm_data_t), IPC_CREAT | 0660);
    if (*shmid < 0) return -1;

    *shm_ptr = (shm_data_t*) shmat(*shmid, NULL, 0);
    if (*shm_ptr == (void*) -1) return -2;

    memset(*shm_ptr, 0, sizeof(shm_data_t));
    (*shm_ptr)->max_pojemnosc = capacity;

    return 0;
}

int init_semaphores(int *semid) {
    key_t key = ftok(".", 75);
    *semid = semget(key, SEM_COUNT, IPC_CREAT | 0660);
    if (*semid < 0) return -1;

    semctl(*semid, SEM_MAGAZYN, SETVAL, 1);
    semctl(*semid, SEM_KLIENCI, SETVAL, 1);

    return 0;
}

int init_message_queue(int *msgid) {
    key_t key = ftok(".", 85);
    *msgid = msgget(key, IPC_CREAT | 0660);
    return *msgid < 0 ? -1 : 0;
}

void cleanup_ipc(int shmid, int semid, int msgid, shm_data_t *shm_ptr) {
    if (shm_ptr) shmdt(shm_ptr);
    if (shmid >= 0) shmctl(shmid, IPC_RMID, NULL);
    if (semid >= 0) semctl(semid, 0, IPC_RMID);
    if (msgid >= 0) msgctl(msgid, IPC_RMID, NULL);
}
