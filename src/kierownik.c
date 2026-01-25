#include "ciastkarnia.h"

// Flaga sterująca pętlą - volatile sig_atomic_t dla bezpieczeństwa wątkowego
volatile sig_atomic_t kontynuuj = 1;

int shmid, semid, msqid;
shared_data_t *shm;

void usun_stare_zasoby() {
    key_t klucz = ftok(".", PROJEKT_ID);
    int s_id = shmget(klucz, 0, 0);
    if (s_id != -1) shmctl(s_id, IPC_RMID, NULL);
    int sem_id = semget(klucz, 0, 0);
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    int m_id = msgget(klucz, 0);
    if (m_id != -1) msgctl(m_id, IPC_RMID, NULL);
}

void raport_do_pliku(shared_data_t *data) {
    FILE *f = fopen("raport_koncowy.txt", "w");
    if(!f) return;
    fprintf(f, "--- RAPORT KOŃCOWY Z SYMULACJI ---\n");
    fprintf(f, "%-15s | %-12s | %-10s | %-10s\n", "Produkt", "Wytworzono", "Sprzedano", "Zostalo");
    fprintf(f, "------------------------------------------------------------\n");
    for (int i = 0; i < P_TYPY; i++) {
        fprintf(f, "%-15s | %-12d | %-10d | %-10d\n", 
               PRODUKTY_NAZWY[i], data->wytworzono[i], data->sprzedano[i], data->stan_podajnika[i]);
    }
    fclose(f);
}

void handler_kierownika(int sig) {
    if (sig == SIGINT) {
        kontynuuj = 0;
    }
}

int main(int argc, char *argv[]) {
    // ARGUMENTY: 1 - czas trwania, 2 - próg otwarcia nowej kasy
    int czas_trwania = (argc > 1) ? atoi(argv[1]) : 30;
    int prog_kasy = (argc > 2) ? atoi(argv[2]) : K_PROG;
    
    srand(time(NULL));
    usun_stare_zasoby();

    key_t klucz = ftok(".", PROJEKT_ID);
    shmid = shmget(klucz, sizeof(shared_data_t), IPC_CREAT | 0666);
    shm = (shared_data_t *)shmat(shmid, NULL, 0);
    
    // Inicjalizacja pamięci
    shm->sklep_otwarty = 0;
    for(int i=0; i<P_TYPY; i++) {
        shm->stan_podajnika[i] = 0;
        shm->wytworzono[i] = 0;
        shm->sprzedano[i] = 0;
    }

    semid = semget(klucz, 2, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, MAX_KLIENCI);
    semctl(semid, 1, SETVAL, 1);
    msqid = msgget(klucz, IPC_CREAT | 0666);

    // Rejestracja SIGINT
    struct sigaction sa;
    sa.sa_handler = handler_kierownika;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    
    signal(SIGCHLD, SIG_IGN); 

    // Start Piekarza
    if (fork() == 0) {
        execl("./piekarz", "piekarz", NULL);
        _exit(1); 
    }
    
    for(int i=0; i<2 && kontynuuj; i++) sleep(1);

    if (kontynuuj) {
        shm->sklep_otwarty = 1; 
        if (fork() == 0) { execl("./kasjer", "kasjer", "1", NULL); _exit(1); }
        printf("[Kierownik] Sklep OTWARTY (czas: %ds, prog: %d)\n", czas_trwania, prog_kasy);
    }

    time_t start_otwarcia = time(NULL);
    int kasa2_otwarta = 0;

    while (kontynuuj && (time(NULL) - start_otwarcia < czas_trwania)) {
        // Logika skalowania: sprawdź liczbę osób w sklepie
        int obecni = MAX_KLIENCI - semctl(semid, 0, GETVAL);
        if (!kasa2_otwarta && obecni >= prog_kasy) {
            printf("[Kierownik] Duży ruch (%d osób). Otwieram Kasę 2.\n", obecni);
            if (fork() == 0) { execl("./kasjer", "kasjer", "2", NULL); _exit(1); }
            kasa2_otwarta = 1;
        }

        if (rand() % 10 < 4) {
            if (fork() == 0) { execl("./klient", "klient", NULL); _exit(1); }
        }
        usleep(700000); 
    }

    printf("\n[Kierownik] Zamykanie sklepu...\n");
    shm->sklep_otwarty = 0; 

    // Czekaj na wyjście klientów
    while ((MAX_KLIENCI - semctl(semid, 0, GETVAL)) > 0 && kontynuuj) {
        sleep(1);
    }

    raport_do_pliku(shm);
    system("pkill -TERM piekarz");
    system("pkill -TERM kasjer");
    
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    msgctl(msqid, IPC_RMID, NULL);

    printf("[Kierownik] Raport gotowy. Do widzenia.\n");
    return 0;
}