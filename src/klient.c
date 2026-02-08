#include "ciastkarnia.h"

int main() {
    srand(getpid() ^ time(NULL));
    key_t key = ftok(".", PROJECT_ID);
    int semid = semget(key, 5, 0666);
    int shmid = shmget(key, sizeof(shared_data_t), 0666);
    int msgid = msgget(key, 0666);
    shared_data_t *data = shmat(shmid, NULL, 0);

    // Czekaj na wejście do sklepu
    sem_op(semid, 1, -1); 

    sem_op(semid, 0, -1);
    data->klienci_lacznie++;
    
    int suma_zakupow = 0;
    for (int t = 0; t < P_TYPY; t++) {
        int ile_chce = rand() % 3; // Chce 0, 1 lub 2 sztuki danego typu
        if (data->stan_podajnika[t] >= ile_chce) {
            data->stan_podajnika[t] -= ile_chce;
            suma_zakupow += ile_chce;
        } else {
            // Bierze ile zostało, reszta to porzucone zamówienie
            suma_zakupow += data->stan_podajnika[t];
            data->stan_podajnika[t] = 0;
            data->porzucone_zamowienia++;
        }
    }
    sem_op(semid, 0, 1);

    if (suma_zakupow > 0) {
        msg_t msg = {1, suma_zakupow};
        msgsnd(msgid, &msg, sizeof(msg_t) - sizeof(long), 0);
    }

    // Wyjdź ze sklepu
    sem_op(semid, 1, 1); 

    shmdt(data);
    return 0;
}