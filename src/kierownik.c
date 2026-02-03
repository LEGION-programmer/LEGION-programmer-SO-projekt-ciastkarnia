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
        fprintf(f, "Produkt %02d | S:%d | Z:%d | P:%d |\n", i, 
                shared_data->sprzedano[i], shared_data->stan_podajnika[i], shared_data->porzucono[i]);
    }
    int ok = (w_sum == s_sum + z_sum + p_sum);
    fprintf(f, "Weryfikacja: %s\n", ok ? "OK" : "ERR");
    printf("\n" MAGENTA "=== KONIEC SYMULACJI ===" RESET "\n");
    printf("Bilans: %s (W:%d, S+Z+P:%d)\n", ok ? GREEN"OK"RESET : RED"ERR"RESET, w_sum, s_sum+z_sum+p_sum);
    fclose(f);
}

void sprzataj(int sig) {
    (void)sig; // Ucisza warning: unused parameter
    printf(YELLOW "\n[Kierownik] Zamykanie sklepu, prosze czekac...\n" RESET);
    shared_data->sklep_otwarty = 0;
    if (gen_pid > 0) kill(gen_pid, SIGTERM);

    struct msqid_ds q_buf;
    int timeout = 0;
    if (msgid != -1) {
        do {
            msgctl(msgid, IPC_STAT, &q_buf);
            if (q_buf.msg_qnum > 0) usleep(100000);
        } while (q_buf.msg_qnum > 0 && timeout++ < 50);
    }

    if (piekarz_pid > 0) kill(piekarz_pid, SIGTERM);
    for(int i=0; i<2; i++) if (kasy[i] > 0) kill(kasy[i], SIGTERM);
    
    while (wait(NULL) > 0);
    raport_koncowy();
    
    if (shmid != -1) { shmdt(shared_data); shmctl(shmid, IPC_RMID, NULL); }
    if (semid != -1) semctl(semid, 0, IPC_RMID);
    if (msgid != -1) msgctl(msgid, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) { printf("Uzycie: ./kierownik N K [czas]\n"); exit(1); }
    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    int czas = (argc > 3) ? atoi(argv[3]) : 0;

    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    shmid = shmget(k, sizeof(shared_data_t), IPC_CREAT | 0666);
    shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    memset(shared_data, 0, sizeof(shared_data_t));

    semid = semget(k, 2, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, N);
    semctl(semid, 1, SETVAL, 1);
    msgid = msgget(k, IPC_CREAT | 0666);

    signal(SIGINT, sprzataj);
    if (czas > 0) { signal(SIGALRM, sprzataj); alarm(czas); }

    printf("[Kierownik] Start piekarni. Oczekiwanie 2s...\n");
    if ((piekarz_pid = fork()) == 0) execl("./piekarz", "piekarz", NULL);
    sleep(2); 

    shared_data->sklep_otwarty = 1;
    printf("[Kierownik] Sklep otwarty (N=%d, K=%d).\n", N, K);
    if ((kasy[0] = fork()) == 0) execl("./kasjer", "kasjer", "1", NULL);

    if ((gen_pid = fork()) == 0) {
        while(1) {
            if (fork() == 0) { execl("./klient", "klient", NULL); exit(0); }
            usleep(300000);
            while(waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    while(1) {
        int w_srodku = N - semctl(semid, 0, GETVAL);
        if (w_srodku >= K && kasy[1] == 0) {
            printf(CYAN "[Kierownik] Tlok (%d klientow). Otwieram druga kase.\n" RESET, w_srodku);
            if ((kasy[1] = fork()) == 0) execl("./kasjer", "kasjer", "2", NULL);
        } else if (w_srodku < K && kasy[1] > 0) {
            printf(CYAN "[Kierownik] Luzniej (%d klientow). Zamykam druga kase.\n" RESET, w_srodku);
            kill(kasy[1], SIGUSR1);
            kasy[1] = 0;
        }
        usleep(500000);
    }
    return 0;
}