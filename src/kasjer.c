#include "ciastkarnia.h"

int main(int argc, char *argv[]) {
    int id_kasy = (argc > 1) ? atoi(argv[1]) : 1;
    key_t klucz = ftok(".", PROJEKT_ID);
    int shmid = shmget(klucz, sizeof(shared_data_t), 0666);
    shared_data_t *shm = (shared_data_t *)shmat(shmid, NULL, 0);
    int semid = semget(klucz, 2, 0666);
    int msqid = msgget(klucz, 0666);

    msg_zamowienie_t zam;
    while (shm->sklep_otwarty || msgrcv(msqid, &zam, sizeof(msg_zamowienie_t)-sizeof(long), 1, IPC_NOWAIT) != -1) {
        if (msgrcv(msqid, &zam, sizeof(msg_zamowienie_t)-sizeof(long), 1, IPC_NOWAIT) != -1) {
            
            // OCHRONA STATYSTYK (KROK 2)
            sem_op(semid, 1, -1); // Wejście do sekcji krytycznej
            for (int i = 0; i < P_TYPY; i++) {
                shm->sprzedano[i] += zam.produkty[i];
            }
            sem_op(semid, 1, 1);  // Wyjście
            
            printf("[Kasa %d] Obsluzono klienta %d\n", id_kasy, zam.klient_id);
            usleep(200000);
        }
        
        if (id_kasy == 2 && (MAX_KLIENCI - semctl(semid, 0, GETVAL)) < K_PROG) break;
        usleep(100000);
    }
    shmdt(shm);
    return 0;
}