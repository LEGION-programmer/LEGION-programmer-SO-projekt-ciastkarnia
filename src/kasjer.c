#include "ciastkarnia.h"

int aktywna = 1;

void obsluga_kasjera(int sig) {
    if (sig == SIGUSR1) aktywna = 0; // Inwentaryzacja: przestań brać nowe, ale dokończ pętlę
    else if (sig == SIGUSR2) _exit(0); // Ewakuacja: wyjdź natychmiast
}

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);

    signal(SIGUSR1, obsluga_kasjera);
    signal(SIGUSR2, obsluga_kasjera);
    signal(SIGINT, SIG_IGN);

    key_t k = ftok(".", PROJEKT_ID);
    int msqid = msgget(k, 0);
    int semid = semget(k, 2, 0);
    shared_data_t *shm = shmat(shmget(k, sizeof(shared_data_t), 0), NULL, 0);

    printf("[Kasa %s] Stanowisko otwarte.\n", argv[1]);

    // Pętla działa dopóki kasa aktywna LUB dopóki są wiadomości w kolejce (Inwentaryzacja)
    while (aktywna || msgrcv(msqid, NULL, 0, 1, IPC_NOWAIT | MSG_NOERROR) != -1) {
        order_t zam;
        // Jeśli aktywna - czekaj blokująco. Jeśli nie - bierz tylko co jest (IPC_NOWAIT).
        if (msgrcv(msqid, &zam, sizeof(order_t)-sizeof(long), 1, aktywna ? 0 : IPC_NOWAIT) != -1) {
            sem_op(semid, 1, -1);
            if (shm->stan_podajnika[zam.typ] >= zam.ilosc) {
                shm->stan_podajnika[zam.typ] -= zam.ilosc;
                shm->sprzedano[zam.typ] += zam.ilosc;
                printf("[Kasa %s] Sprzedano %d x %s\n", argv[1], zam.ilosc, PRODUKTY_NAZWY[zam.typ]);
                usleep(800000);
            }
            sem_op(semid, 1, 1);
        }
        // stress test
        if (aktywna) usleep(1000); 
    }

    printf("[Kasa %s] Zamknieta po obsluzeniu kolejki.\n", argv[1]);
    return 0;
}