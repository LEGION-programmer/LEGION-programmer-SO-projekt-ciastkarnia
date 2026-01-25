#include "ciastkarnia.h"

int main() {
    srand(getpid());
    key_t klucz = ftok(".", PROJEKT_ID);
    int shmid = shmget(klucz, sizeof(shared_data_t), 0666);
    shared_data_t *shm = (shared_data_t *)shmat(shmid, NULL, 0);
    int semid = semget(klucz, 2, 0666);

    while (shm->sklep_otwarty || shm->wytworzono[0] == 0) {
        usleep(1500000); 
        
        int typ = rand() % P_TYPY;
        int ile = (rand() % 5) + 1;

        sem_op(semid, 1, -1); // WEJŚCIE DO SEKCJI KRYTYCZNEJ
        
        // Sprawdzamy, czy zmieści się na podajniku
        if (shm->stan_podajnika[typ] + ile <= POJEMNOSC_PODAJNIKA) {
            shm->stan_podajnika[typ] += ile;
            shm->wytworzono[typ] += ile; // Statystyka rośnie TYLKO gdy faktycznie dostarczono
            printf("[Piekarz] Dostawa: %s (+%d)\n", PRODUKTY_NAZWY[typ], ile);
        } else {
            // Jeśli podajnik pełny, piekarz dodaje tylko tyle, ile wejdzie
            int wolne = POJEMNOSC_PODAJNIKA - shm->stan_podajnika[typ];
            if (wolne > 0) {
                shm->stan_podajnika[typ] += wolne;
                shm->wytworzono[typ] += wolne;
                printf("[Piekarz] Dostawa czesciowa: %s (+%d)\n", PRODUKTY_NAZWY[typ], wolne);
            }
        }
        
        sem_op(semid, 1, 1); // WYJŚCIE
        
        if (!shm->sklep_otwarty && shm->wytworzono[0] > 0) break;
    }
    shmdt(shm);
    return 0;
}