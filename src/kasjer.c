#include "ciastkarnia.h"

int main(int argc, char *argv[]) {
    int id = (argc > 1) ? atoi(argv[1]) : 1;
    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    int shmid = shmget(k, sizeof(shared_data_t), 0666);
    int semid = semget(k, 2, 0666);
    int msqid = msgget(k, 0666);
    shared_data_t *shm = shmat(shmid, NULL, 0);
    order_t zam;

    printf("[Kasjer %d] Stanowisko otwarte.\n", id);

    while(1) {
        if (msgrcv(msqid, &zam, MSG_SIZE, 1, 0) != -1) {
            printf("[Kasjer %d] Obsluguje klienta %d (%d szt.)\n", id, zam.klient_pid, zam.liczba_pozycji);
            
            sem_op(semid, 1, -1); // LOCK
            for(int i = 0; i < zam.liczba_pozycji; i++) {
                int produkt_id = zam.typy[i];
                shm->sprzedano[produkt_id]++; // AKTUALIZACJA W SHM
            }
            sem_op(semid, 1, 1); // UNLOCK
            fflush(stdout);
        } else {
            break; 
        }
    }
    return 0;
}