#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#define _POSIX_C_SOURCE 200809L

#include <signal.h>

/* ===== DANE WSPÓLNE ===== */
typedef struct {
    int ciastka;
    int max;

    int wyprodukowane;
    int sprzedane;

    int klienci_w_sklepie;
    int max_klientow;

    int obsluzeni_klienci;
} shared_data_t;

/* ===== WIADOMOŚĆ KLIENTA ===== */
typedef struct {
    long mtype;
    int client_id;
    int ile_chce;
} client_msg_t;

/* ===== STEROWANIE ===== */
extern volatile sig_atomic_t running;
void sigint_handler(int sig);

/* ===== IPC ===== */
void init_shared_memory(shared_data_t **shm, int *shm_id,
                        int max_magazyn, int max_klientow);
void cleanup_shared_memory(int shm_id, shared_data_t *shm);

int init_semaphore();
void sem_down(int semid);
void sem_up(int semid);

int init_queue();

/* ===== PROCESY ===== */
void proces_piekarz(int id, shared_data_t *shm, int semid);
void proces_kasjer(int id, shared_data_t *shm, int semid, int msgid);
void proces_klient(int id, shared_data_t *shm, int semid, int msgid);

#endif
