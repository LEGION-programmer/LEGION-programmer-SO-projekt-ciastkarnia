#include "ciastkarnia.h"
#include <sys/wait.h>

int shmid = -1, semid = -1, msqid = -1;
shared_data_t *shm = (void *)-1;
pid_t kasy[2] = {0, 0};

void generuj_raport() {
    if (shm == (void *)-1) return;
    FILE *f = fopen("raport.txt", "w");
    #define PRINT_BOTH(...) { printf(__VA_ARGS__); if(f) fprintf(f, __VA_ARGS__); }

    PRINT_BOTH("\n====================================================\n");
    PRINT_BOTH("         RAPORT KONCOWY Z PRACY CIASTKARNI\n");
    PRINT_BOTH("====================================================\n");
    PRINT_BOTH("%-15s | %-12s | %-10s | %-8s\n", "Produkt", "Wytworzono", "Sprzedano", "Zostalo");
    PRINT_BOTH("----------------------------------------------------\n");

    int sw = 0, ss = 0, sz = 0;
    for (int i = 0; i < P_TYPY; i++) {
        int w = shm->wytworzono[i];
        int s = shm->sprzedano[i];
        int z = shm->stan_podajnika[i];
        PRINT_BOTH("%-15s | %-12d | %-10d | %-8d\n", PRODUKTY_NAZWY[i], w, s, z);
        sw += w; ss += s; sz += z;
    }
    PRINT_BOTH("----------------------------------------------------\n");
    PRINT_BOTH("%-15s | %-12d | %-10d | %-8d\n", "SUMA", sw, ss, sz);
    PRINT_BOTH("Weryfikacja: %d (W) - %d (S) = %d (Z) -> %s\n", sw, ss, sw-ss, (sw-ss == sz) ? "OK" : "BLAD");
    PRINT_BOTH("====================================================\n");
    if (f) { fflush(f); fclose(f); }
}

void sprzatanie_ipc(int sig) {
    // BLOKADA: Kierownik całkowicie ignoruje te sygnały u siebie
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGINT,  SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if (sig == SIGINT || sig == SIGUSR2) {
        printf("\n[Kierownik] !!! EWAKUACJA (Sygnal SIGUSR2) !!!\n");
        // kill(-getpid(), ...) wysyła sygnał do całej grupy procesów
        // Używamy tego zamiast 0, aby być bardziej precyzyjnym
        kill(0, SIGUSR2);
    } else {
        printf("\n[Kierownik] Zamykanie sklepu - Inwentaryzacja (Sygnal SIGUSR1)\n");
        kill(0, SIGUSR1);
    }

    // Dajemy czas dzieciom na zapisanie danych w SHM
    usleep(500000); 

    generuj_raport();

    if (shm != (void *)-1) shmdt(shm);
    if (shmid != -1) shmctl(shmid, IPC_RMID, NULL);
    if (semid != -1) semctl(semid, 0, IPC_RMID);
    if (msqid != -1) msgctl(msqid, IPC_RMID, NULL);
    
    printf("[Kierownik] Zasoby IPC usuniete. Koniec.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    // Ustawienie grupy procesów - kluczowe dla poprawnego kill(0, ...)
    setpgid(0, 0); 

    int N = (argc > 1) ? atoi(argv[1]) : 10;
    int K = (argc > 2) ? atoi(argv[2]) : N / 2;
    int stress = (argc > 3) ? atoi(argv[3]) : 0;
    int sym_czas = 20;

    key_t klucz = ftok(".", PROJEKT_ID);
    shmid = shmget(klucz, sizeof(shared_data_t), IPC_CREAT | 0600);
    semid = semget(klucz, 2, IPC_CREAT | 0600);
    msqid = msgget(klucz, IPC_CREAT | 0600);
    shm = shmat(shmid, NULL, 0);

    if (shm != (void*)-1) memset(shm, 0, sizeof(shared_data_t));
    
    semctl(semid, 0, SETVAL, N); 
    semctl(semid, 1, SETVAL, 1); 

    signal(SIGINT, sprzatanie_ipc);
    signal(SIGTERM, sprzatanie_ipc);
    signal(SIGUSR1, sprzatanie_ipc);

    if (fork() == 0) execl("./piekarz", "piekarz", NULL);
    sleep(1); 

    shm->sklep_otwarty = 1;
    time_t start = time(NULL);

    while (stress > 0 || (time(NULL) - start < sym_czas)) {
        int obecni = N - semctl(semid, 0, GETVAL);
        int potrzebne_kasy = (K > 0) ? (obecni / K) + 1 : 1;
        if (potrzebne_kasy > 2) potrzebne_kasy = 2; 

        for (int i = 0; i < 2; i++) {
            if (i < potrzebne_kasy && kasy[i] == 0) {
                char nr[2]; sprintf(nr, "%d", i+1);
                if ((kasy[i] = fork()) == 0) execl("./kasjer", "kasjer", nr, NULL);
            }
        }

        if (stress > 0) {
            if (fork() == 0) { execl("./klient", "klient", NULL); exit(0); }
            stress--;
            usleep(100); 
        } else {
            if (rand() % 10 < 3) { 
                if (fork() == 0) { execl("./klient", "klient", NULL); exit(0); } 
            }
            sleep(1);
        }
    }

    sprzatanie_ipc(SIGUSR1);
    return 0;
}