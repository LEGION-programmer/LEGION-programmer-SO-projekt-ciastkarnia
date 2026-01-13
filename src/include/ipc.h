#ifndef IPC_H
#define IPC_H

#include "ciastkarnia.h"

int init_shared_memory(int *shmid, shared_data_t **shm_ptr, int max_magazyn);
void cleanup_ipc(int shmid, int semid, int msgid, shared_data_t *shm_ptr);
int init_semaphores(int n);
void sem_up(int semid);
void sem_down(int semid);
int init_message_queue(void);

#endif
