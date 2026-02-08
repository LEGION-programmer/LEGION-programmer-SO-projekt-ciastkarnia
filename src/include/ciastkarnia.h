#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#define PROJECT_ID 'C'
#define P_TYPY 3

typedef struct {
    int stan_podajnika[P_TYPY];
    int sprzedane_ciastka;
    int porzucone_zamowienia;
    int wyprodukowane_lacznie;
    int klienci_lacznie;
} shared_data_t;

typedef struct {
    long mtype;
    int laczna_liczba_ciastek; // Klient podaje sumę zakupów z koszyka
} msg_t;

static inline void sem_op(int semid, int semnum, int op) {
    struct sembuf s = { (unsigned short)semnum, (short)op, 0 };
    while (semop(semid, &s, 1) == -1) {
        if (errno != EINTR) break;
    }
}

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"

#endif