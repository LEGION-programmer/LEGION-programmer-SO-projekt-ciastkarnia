#ifndef IPC_H
#define IPC_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define MAX_PRODUCTS 20

// ----- Shared Memory Structure -----
typedef struct {
    int magazyn[MAX_PRODUCTS]; // ilość produktów
    int max_pojemnosc;         // pojemność magazynu
    int klienci_w_sklepie;     // aktualna liczba klientów
} shm_data_t;

// ----- Message Queue -----
typedef struct {
    long mtype;       // typ wiadomości (np. ID klienta)
    int client_id;    // ID klienta
    int products[MAX_PRODUCTS]; // lista zakupów
} client_msg_t;

// ----- SEMAPHORE indices -----
enum {
    SEM_MAGAZYN = 0,
    SEM_KLIENCI = 1,
    SEM_COUNT
};

// ----- Functions -----
int init_shared_memory(int *shmid, shm_data_t **shm_ptr, int capacity);
int init_semaphores(int *semid);
int init_message_queue(int *msgid);

void cleanup_ipc(int shmid, int semid, int msgid, shm_data_t *shm_ptr);

#endif
