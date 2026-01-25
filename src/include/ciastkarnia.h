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
#include <errno.h>

#define PROJEKT_ID 99
#define P_TYPY 11
#define MAX_KLIENCI 10  // N
#define K_PROG (MAX_KLIENCI/2) // K
#define POJEMNOSC_PODAJNIKA 15

// Typy sygnałów z zadania
#define SIG_INWENTARYZACJA SIGUSR1
#define SIG_EWAKUACJA SIGUSR2

const char *PRODUKTY_NAZWY[] = {
    "Sernik", "Makowiec", "Paczki", "Ekler", "Muffin", 
    "Beza", "Szarlotka", "Wuzetka", "Rogal", "Tarta", "Pralina"
};

typedef struct {
    int stan_podajnika[P_TYPY];
    int wytworzono[P_TYPY];
    int sprzedano[P_TYPY];
    int sklep_otwarty;
    int ewakuacja; // Flaga dla klientów
} shared_data_t;

typedef struct {
    long mtype;
    int klient_id;
    int produkty[P_TYPY];
} msg_zamowienie_t;

void sem_op(int semid, int sem_num, int op) {
    struct sembuf sb = {sem_num, op, 0};
    semop(semid, &sb, 1);
}

#endif