#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define PROJEKT_ID 99
#define P_TYPY 11
#define MAX_KLIENCI_W_SRODKU 5
#define POJEMNOSC_PODAJNIKA 10

static const char* PRODUKTY_NAZWY[] = {
    "Paczek", "Kremowka", "WZ-tka", "Sernik", "Makowiec", 
    "Ekler", "Mazurek", "Drozdżowka", "Beza", "Tarta", "Szarlotka"
};

typedef struct {
    int stan_podajnika[P_TYPY];
    int wytworzono[P_TYPY];
    int sprzedano[P_TYPY];
    int sklep_otwarty;
} shared_data_t;

typedef struct {
    long mtype;
    int klient_id;
    int produkty[P_TYPY];
} msg_zamowienie_t;

static void sem_op(int semid, int sem_num, int op) {
    struct sembuf sb = {sem_num, op, 0};
    semop(semid, &sb, 1);
}

#endif