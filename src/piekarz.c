#include "ciastkarnia.h"

void stop_piekarz(int sig) {
    (void)sig; 
    _exit(0); 
}

int main() {
    signal(SIGUSR1, stop_piekarz);
    signal(SIGUSR2, stop_piekarz);
    signal(SIGINT, SIG_IGN);

    key_t k = ftok(".", PROJEKT_ID);
    int shmid = shmget(k, sizeof(shared_data_t), 0);
    shared_data_t *shm = shmat(shmid, NULL, 0);
    int semid = semget(k, 2, 0);

    srand(getpid());

    printf("[Piekarz] Rozpoczynam wypieki...\n");

    while(1) {
        int typ = rand() % P_TYPY;
        int ilosc = (rand() % 3) + 1;

        sem_op(semid, 1, -1); // Wejście do sekcji krytycznej
        
        if (shm->stan_podajnika[typ] + ilosc <= POJEMNOSC_PODAJNIKA) {
            shm->stan_podajnika[typ] += ilosc;
            shm->wytworzono[typ] += ilosc;
            
            // DODANO: Komunikat o wypieku
            printf("[Piekarz] Wypieczono: %d x %s. Stan na polce: %d\n", 
                   ilosc, PRODUKTY_NAZWY[typ], shm->stan_podajnika[typ]);
        }
        
        sem_op(semid, 1, 1); // Wyjście z sekcji krytycznej

        // SPOWOLNIENIE: 
        usleep(800000); 
    }
    return 0;
}