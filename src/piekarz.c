#include "ciastkarnia.h"

int main() {
    key_t klucz = ftok(".", PROJECT_ID);
    int semid = semget(klucz, 5, 0666);
    int shmid = shmget(klucz, sizeof(shared_data_t), 0666);
    shared_data_t *data = shmat(shmid, NULL, 0);

    while(1) {
        usleep(30000); // domyslnie 30000 zwiekszyc dla testu awarii pieca np. 300000000
        int t = rand() % P_TYPY;
        sem_op(semid, 0, -1);
        data->stan_podajnika[t] += 100; // Hurtowe ilości
        data->wyprodukowane_lacznie += 100;
        sem_op(semid, 0, 1);
    }
    return 0;
}