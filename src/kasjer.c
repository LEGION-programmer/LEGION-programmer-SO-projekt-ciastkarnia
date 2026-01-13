#include "include/ciastkarnia.h"

int main(int argc, char* argv[]) {
    int kasa_id = (argc > 1) ? atoi(argv[1]) : 1;
    key_t key = ftok(".", PROJEKT_ID);
    int shmid = shmget(key, sizeof(shared_data_t), 0600);
    shared_data_t *shm = (shared_data_t*)shmat(shmid, NULL, 0);
    int semid = semget(key, 2, 0600);
    int msgid = msgget(key, 0600);

    msg_zamowienie_t zam;
    while (shm->sklep_otwarty) {
        if (msgrcv(msgid, &zam, sizeof(zam) - sizeof(long), 1, IPC_NOWAIT) != -1) {
            printf("[Kasa %d] Obsługuje klienta %d.\n", kasa_id, zam.klient_id);
            sem_op(semid, 1, -1);
            for (int i = 0; i < P_TYPY; i++) {
                shm->sprzedano[i] += zam.produkty[i];
            }
            sem_op(semid, 1, 1);
            usleep(300000);
        }
        usleep(100000);
    }
    shmdt(shm);
    return 0;
}