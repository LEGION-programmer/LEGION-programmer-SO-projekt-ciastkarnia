#include "ciastkarnia.h"

int main() {
    srand(time(NULL) ^ getpid());
    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    int shmid = shmget(k, sizeof(shared_data_t), 0666);
    int semid = semget(k, 2, 0666);
    shared_data_t *shm = shmat(shmid, NULL, 0);

    printf("[Piekarz] Rozpoczynam wypieki (produkcja seryjna)!\n");

    while(1) {
        sem_op(semid, 1, -1); // Lock podajnika
        
        printf("[Piekarz] Wyjmuję z pieca: ");
        for (int i = 0; i < P_TYPY; i++) {
            int ilosc = (rand() % 5) + 1; // Losowo 1-5 sztuk każdego typu
            shm->stan_podajnika[i] += ilosc;
            shm->wytworzono[i] += ilosc;
            if (i < 3) printf("%s (+%d) ", PRODUKTY_NAZWY[i], ilosc);
        }
        printf("... i inne.\n");
        
        sem_op(semid, 1, 1); // Unlock
        
        // Odstęp między pieczeniem kolejnej partii
        sleep(2); 
    }
    return 0;
}