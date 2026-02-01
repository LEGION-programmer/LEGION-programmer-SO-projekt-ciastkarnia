#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define PROJEKT_ID 'C'
#define P_TYPY 11
#define POJEMNOSC_PODAJNIKA 20

typedef struct {
    int sklep_otwarty;
    int stan_podajnika[P_TYPY];
    int wytworzono[P_TYPY];
    int sprzedano[P_TYPY];
} shared_data_t;

typedef struct {
    long mtype;
    int typ;
    int ilosc;
} order_t;

static const char *PRODUKTY_NAZWY[] = {
    "Sernik", "Makowiec", "Paczki", "Drozdzowka", "Beza", 
    "Mazurek", "Szarlotka", "Kremowka", "Piernik", "Tarta", "Eklery"
};

// Pomocnicza funkcja do semaforów
static inline int sem_op(int semid, int sem_num, int op) {
    struct sembuf sb = {sem_num, (short)op, SEM_UNDO};
    return semop(semid, &sb, 1);
}

#endif