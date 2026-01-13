#include "include/ciastkarnia.h"

int main() {
    key_t key = ftok(".", PROJEKT_ID);
    int shmid = shmget(key, sizeof(shared_data_t), 0600);
    shared_data_t *shm = (shared_data_t*)shmat(shmid, NULL, 0);
    int semid = semget(key, 2, 0600);

    while (shm->sklep_otwarty) {
        int typ = rand() % P_TYPY;
        usleep(500000); // Czas pieczenia

        sem_op(semid, 1, -1);
        if (shm->stan_podajnika[typ] < POJEMNOSC_PODAJNIKA) {
            shm->stan_podajnika[typ]++;
            shm->wytworzono[typ]++;
            printf("[Piekarz] +1 %s (W magazynie: %d)\n", PRODUKTY_NAZWY[typ], shm->stan_podajnika[typ]);
        }
        sem_op(semid, 1, 1);
    }
    shmdt(shm);
    return 0;
}