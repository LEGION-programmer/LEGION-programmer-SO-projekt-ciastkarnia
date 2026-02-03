#ifndef CIASTKARNIA_H
#define CIASTKARNIA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define PROJEKT_ID 'C'
#define FTOK_PATH "/tmp"
#define P_TYPY 11

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

static const char *PRODUKTY_NAZWY[] = {
    "Sernik", "Makowiec", "Paczki", "Drozdzowka", "Beza",
    "Mazurek", "Szarlotka", "Kremowka", "Piernik", "Tarta", "Eklery"
};

typedef struct {
    int sklep_otwarty;
    int stan_podajnika[P_TYPY];
    int wytworzono[P_TYPY];
    int sprzedano[P_TYPY];
    int porzucono[P_TYPY];
} shared_data_t;

typedef struct {
    long mtype;
    int klient_pid;
    int typy[10];
    int ilosci[10];
    int liczba_pozycji;
} order_t;

#define MSG_SIZE (sizeof(order_t) - sizeof(long))

void handle_error(const char *msg);
int sem_op(int semid, int sem_num, int op);

#endif