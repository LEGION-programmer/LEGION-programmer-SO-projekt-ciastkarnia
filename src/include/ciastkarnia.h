#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <signal.h>

/* ===== SHARED MEMORY ===== */
typedef struct {
    int ciastka;
    int max_ciasta;

    int kolejka;

    int klienci_przyszli;
    int klienci_obsluzeni;
    int sprzedane_ciasta;
} shared_data_t;

/* ===== MESSAGE ===== */
typedef struct {
    long mtype;
    int client_id;
    int ile_chce;
} client_msg_t;

/* ===== IPC ===== */
void init_shared_memory(shared_data_t **shm, int *shmid, int max);
void cleanup_shared_memory(int shmid, shared_data_t *shm);

int init_semaphore();
void sem_down(int semid);
void sem_up(int semid);

int init_queue();

/* ===== PROCESY ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid);
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid);
void proces_klient(int id, int msgid);

/* ===== SIGNAL ===== */
void sigint_handler(int sig);

#endif
