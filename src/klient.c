#include "include/ciastkarnia.h"

int main(int argc, char* argv[]) {
    if (argc < 2) exit(1);
    int id = atoi(argv[1]);
    srand(time(NULL) ^ (getpid() << 16));

    key_t key = ftok(".", PROJEKT_ID);
    int shmid = shmget(key, sizeof(shared_data_t), 0600);
    shared_data_t *shm = (shared_data_t*)shmat(shmid, NULL, 0);
    int semid = semget(key, 2, 0600);
    int msgid = msgget(key, 0600);

    sem_op(semid, 0, -1);
    printf("[Klient %d] Wszedł do sklepu.\n", id);

    msg_zamowienie_t zam;
    zam.mtype = 1;
    zam.klient_id = id;
    memset(zam.produkty, 0, sizeof(zam.produkty));
    int suma_produktow = 0;

    for(int i = 0; i < 3; i++) {
        int typ = rand() % P_TYPY;
        for(int r = 0; r < 2; r++) { // 2 próby czekania
            sem_op(semid, 1, -1);
            if (shm->stan_podajnika[typ] > 0) {
                shm->stan_podajnika[typ]--;
                zam.produkty[typ]++;
                suma_produktow++;
                printf("  -> [Klient %d] Kupuje: %s\n", id, PRODUKTY_NAZWY[typ]);
                sem_op(semid, 1, 1);
                break;
            }
            sem_op(semid, 1, 1);
            usleep(400000);
        }
    }

    if (suma_produktow > 0) {
        printf("[Klient %d] Idzie do kasy z produktami (sztuk: %d).\n", id, suma_produktow);
        msgsnd(msgid, &zam, sizeof(zam) - sizeof(long), 0);
        usleep(500000); // Płacenie
        printf("[Klient %d] Zapłacił i wychodzi.\n", id);
    } else {
        printf("[Klient %d] Nic nie kupił i po prostu wychodzi.\n", id);
    }

    sem_op(semid, 0, 1);
    shmdt(shm);
    return 0;
}