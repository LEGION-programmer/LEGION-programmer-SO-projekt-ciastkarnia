#include "ciastkarnia.h"

int main() {
    srand(time(NULL) ^ getpid());
    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    int shmid = shmget(k, sizeof(shared_data_t), 0666);
    int semid = semget(k, 2, 0666);
    int msqid = msgget(k, 0666);

    shared_data_t *shm = shmat(shmid, NULL, 0);

    // Klient wchodzi
    if (shm->sklep_otwarty && sem_op(semid, 0, -1) == 0) {
        // --- KLUCZ DO TESTU 2 ---
        // Klient musi pobyć w sklepie, żeby kolejni zdążyli wejść
        // i stworzyć "tłok" widoczny dla Kierownika.
        usleep(600000); 

        order_t zam;
        memset(&zam, 0, sizeof(order_t));
        zam.mtype = 1;
        zam.klient_pid = getpid();

        for (int i = 0; i < 2; i++) {
            int t = rand() % P_TYPY;
            sem_op(semid, 1, -1);
            if (shm->stan_podajnika[t] > 0) {
                shm->stan_podajnika[t]--;
                zam.typy[zam.liczba_pozycji] = t;
                zam.ilosci[zam.liczba_pozycji] = 1;
                printf("[Klient %d] Kupuję: %s\n", getpid(), PRODUKTY_NAZWY[t]);
                zam.liczba_pozycji++;
            }
            sem_op(semid, 1, 1);
        }

        if (zam.liczba_pozycji > 0) {
            msgsnd(msqid, &zam, MSG_SIZE, 0);
        }
        
        // Klient wychodzi, zwalnia miejsce (Semafor 0)
        sem_op(semid, 0, 1);
    }
    shmdt(shm);
    return 0;
}