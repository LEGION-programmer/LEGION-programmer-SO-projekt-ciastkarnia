#include "ciastkarnia.h"

pid_t piekarz_pid = 0, kasy[2] = {0, 0}, gen_pid = 0;
int shmid = -1, semid = -1, msgid = -1;
shared_data_t *shared_data = NULL;

void raport_koncowy() {
    FILE *f = fopen("raport.txt", "w");
    if (!f) return;
    int w_sum = 0, s_sum = 0, z_sum = 0, p_sum = 0;
    for (int i = 0; i < P_TYPY; i++) {
        w_sum += shared_data->wytworzono[i];
        s_sum += shared_data->sprzedano[i];
        z_sum += shared_data->stan_podajnika[i];
        p_sum += shared_data->porzucono[i];
        fprintf(f, "Produkt %02d | S: %d |\n", i, shared_data->sprzedano[i]);
    }
    fprintf(f, "Weryfikacja: %s\n", (w_sum == s_sum + z_sum + p_sum) ? "OK" : "ERR");
    fclose(f);
}

void sprzataj(int sig) {
    if (shared_data) shared_data->sklep_otwarty = 0;
    if (gen_pid > 0) kill(gen_pid, SIGTERM);
    
    // Czekamy na opróżnienie kolejki (ważne dla Testu 1)
    struct msqid_ds buf;
    int limit = 0;
    if (msgid != -1) {
        do {
            msgctl(msgid, IPC_STAT, &buf);
            if (buf.msg_qnum > 0) usleep(100000);
            limit++;
        } while (buf.msg_qnum > 0 && limit < 50);
    }

    if (piekarz_pid > 0) kill(piekarz_pid, SIGTERM);
    if (kasy[0] > 0) kill(kasy[0], SIGTERM);
    if (kasy[1] > 0) kill(kasy[1], SIGTERM);
    while (wait(NULL) > 0);
    
    raport_koncowy();
    if (shmid != -1) { shmdt(shared_data); shmctl(shmid, IPC_RMID, NULL); }
    if (semid != -1) semctl(semid, 0, IPC_RMID);
    if (msgid != -1) msgctl(msgid, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) exit(1);
    int N = atoi(argv[1]);
    int K = atoi(argv[2]);

    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    shmid = shmget(k, sizeof(shared_data_t), IPC_CREAT | 0666);
    shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    memset(shared_data, 0, sizeof(shared_data_t));

    semid = semget(k, 2, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, N);
    semctl(semid, 1, SETVAL, 1);
    msgid = msgget(k, IPC_CREAT | 0666);

    signal(SIGINT, sprzataj);
    signal(SIGALRM, sprzataj);

    // 1. Piekarz (Tp)
    if ((piekarz_pid = fork()) == 0) execl("./piekarz", "piekarz", NULL);
    
    // 2. Czekanie 3s (Tp + 30min symulacji)
    sleep(3); 

    // 3. Otwarcie sklepu
    shared_data->sklep_otwarty = 1;
    if ((kasy[0] = fork()) == 0) execl("./kasjer", "kasjer", "1", NULL);

    // 4. Generator (do testów ręcznych)
    if ((gen_pid = fork()) == 0) {
        while(1) {
            if (fork() == 0) { execl("./klient", "klient", NULL); exit(0); }
            usleep(500000);
        }
    }

    // --- LOGIKA DYNAMIKI KAS (KLUCZ DO TESTU 2) ---
    
    while(1) {
        int wolne = semctl(semid, 0, GETVAL);
        if (wolne == -1) break;
        int w_srodku = N - wolne;

        if (w_srodku >= K && kasy[1] == 0) {
            kasy[1] = fork();
            if (kasy[1] == 0) {
                // Upewnij się, że ścieżka "./kasjer" jest poprawna względem miejsca odpalenia kierownika
                execl("./kasjer", "kasjer", "2", NULL);
                
                // Jeśli execl zawiedzie, wypisze błąd:
                perror("BŁĄD EXECL KASJER 2");
                _exit(1); 
            }
            // Wypisz to dokładnie tak, żebyś widział w konsoli podczas testu
            printf("[Kierownik] PRÓBA OTWARCIA KASY 2 (Klienci: %d)\n", w_srodku);
            fflush(stdout);
            usleep(200000); // Daj systemowi czas na stworzenie procesu
        }
    }
    return 0;
}