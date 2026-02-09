#include "ciastkarnia.h"

int main() {
    // Unikalne ziarno dla każdego procesu klienta
    srand(getpid() ^ time(NULL));

    key_t key = ftok(".", PROJECT_ID);
    int semid = semget(key, 5, 0666);
    int shmid = shmget(key, sizeof(shared_data_t), 0666);
    int msgid = msgget(key, 0666);
    shared_data_t *data = shmat(shmid, NULL, 0);

    // --- 1. WEJŚCIE DO SKLEPU ---
    // Czekaj na wolne miejsce (semafor nr 1)
    sem_op(semid, 1, -1); 

    // Sekcja krytyczna: zwiększamy licznik obecnych i łączny licznik klientów
    sem_op(semid, 0, -1);
    data->klienci_lacznie++;
    sem_op(semid, 0, 1);

    // Symulacja czasu potrzebnego na dojście do lady
    //usleep(100000); // 0.1s odkomentowac dla testu bottleck

    // --- 2. ZAKUPY (PRZY LADZIE) ---
    int suma_zakupow = 0;
    
    // Blokujemy ladę na czas całych zakupów jednego klienta
    sem_op(semid, 0, -1); 
    for (int t = 0; t < P_TYPY; t++) {
        // Losujemy: klient chce od 1 do 3 ciastek danego typu
        int ile_chce = (rand() % 3) + 2; 

        if (data->stan_podajnika[t] >= ile_chce) {
            // Sukces: bierze tyle ile chciał
            data->stan_podajnika[t] -= ile_chce;
            suma_zakupow += ile_chce;
        } else {
            // Częściowy sukces lub porażka
            if (data->stan_podajnika[t] > 0) {
                suma_zakupow += data->stan_podajnika[t];
                data->stan_podajnika[t] = 0;
            }
            // Odnotowujemy, że klient nie dostał wszystkiego co chciał
            data->porzucone_zamowienia++;
        }
    }
    sem_op(semid, 0, 1); // Zwalniamy ladę dla innych klientów/piekarza

    // Symulacja czasu na dojście do kasy
    //usleep(50000); // odkomentowac dla testu bottleck

    // --- 3. KASA ---
    if (suma_zakupow > 0) {
        msg_t msg;
        msg.mtype = 1;
        msg.laczna_liczba_ciastek = suma_zakupow;
        
        // Wysyłamy zamówienie do kolejki kasjerów
        msgsnd(msgid, &msg, sizeof(msg_t) - sizeof(long), 0);
        
        // Klient czeka chwilę przy kasie (obsługa trwa)
        //usleep(300000); // odkomentowac dla testu bottleck
    }

    // --- 4. WYJŚCIE ---
    // Zwalniamy miejsce w sklepie (semafor nr 1)
    sem_op(semid, 1, 1); 

    shmdt(data);
    return 0;
}