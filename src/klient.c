#include "ciastkarnia.h"

// Obsługa sygnału ewakuacji (Sygnał 2 z zadania)
void handler_ewakuacja(int sig) {
    (void)sig; 
    printf("[Klient %d] EWAKUACJA! Porzucam zakupy i wychodze.\n", getpid());
    exit(0);
}

int main() {
    // Inicjalizacja generatora liczb losowych unikalna dla każdego procesu
    srand(getpid() ^ time(NULL));

    key_t klucz = ftok(".", PROJEKT_ID);
    int shmid = shmget(klucz, sizeof(shared_data_t), 0666);
    shared_data_t *shm = (shared_data_t *)shmat(shmid, NULL, 0);
    int semid = semget(klucz, 2, 0666);
    int msqid = msgget(klucz, 0666);

    // Rejestracja sygnału ewakuacji
    signal(SIG_EWAKUACJA, handler_ewakuacja);

    // 1. Wejście do sklepu - ograniczona liczba N osób (semafor licznikowy)
    sem_op(semid, 0, -1);
    printf("[Klient %d] Wszedl do sklepu.\n", getpid());

    msg_zamowienie_t zam;
    zam.mtype = 1; 
    zam.klient_id = getpid();
    for(int i=0; i<P_TYPY; i++) zam.produkty[i] = 0;

    int kupione_rodzaje = 0;
    int proby = 0;
    const int MAX_PROB = 10; // "Bezpiecznik" - klient nie będzie czekał wiecznie

    // 2. Proces zakupowy
    // Klient próbuje kupić min. 2 RÓŻNE produkty (wymóg zadania)
    while(kupione_rodzaje < 2 && proby < MAX_PROB && shm->sklep_otwarty) {
        int typ = rand() % P_TYPY;
        
        // Sekcja krytyczna - sprawdzamy i pobieramy towar
        sem_op(semid, 1, -1); 
        if (shm->stan_podajnika[typ] > 0) {
            int do_pobrania = (rand() % 3) + 1; // Klient chce np. 1-3 sztuki
            
            // Pobieramy tylko tyle, ile faktycznie jest na podajniku
            if (do_pobrania > shm->stan_podajnika[typ]) {
                do_pobrania = shm->stan_podajnika[typ];
            }

            shm->stan_podajnika[typ] -= do_pobrania;
            zam.produkty[typ] = do_pobrania;
            
            printf("[Klient %d] Wybral: %s (%d szt.)\n", getpid(), PRODUKTY_NAZWY[typ], do_pobrania);
            kupione_rodzaje++;
        }
        sem_op(semid, 1, 1); // Koniec sekcji krytycznej
        
        proby++;
        if (kupione_rodzaje < 2) {
            usleep(200000); // Krótka pauza, by dać szansę Piekarzowi na dostawę
        }
    }

    // 3. Kasa
    // Jeśli klient cokolwiek kupił, wysyła zamówienie do kolejki IPC
    if (kupione_rodzaje > 0 && shm->sklep_otwarty) {
        if (msgsnd(msqid, &zam, sizeof(msg_zamowienie_t) - sizeof(long), 0) == -1) {
            perror("Błąd wysyłania do kasy");
        } else {
            printf("[Klient %d] Zakupy gotowe, idzie do kasy.\n", getpid());
        }
    } else {
        printf("[Klient %d] Nic nie kupil i wychodzi ze sklepu.\n", getpid());
    }

    // 4. Wyjście ze sklepu - zwolnienie miejsca (semafor N)
    sem_op(semid, 0, 1);
    shmdt(shm);

    return 0;
}