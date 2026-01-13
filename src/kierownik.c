#include "include/ciastkarnia.h"

int shmid, semid, msgid;
shared_data_t *shm = NULL;
pid_t pids[100];
int p_count = 0;

void raport_koncowy() {
    if (!shm) return;
    printf("\n====================================================\n");
    printf("        KOŃCOWY RAPORT SPRZEDAŻY\n");
    printf("====================================================\n");
    printf("%-15s | %-10s | %-10s\n", "Produkt", "Wypieczono", "Sprzedano");
    printf("----------------------------------------------------\n");
    for (int i = 0; i < P_TYPY; i++) {
        printf("%-15s | %-10d | %-10d\n", PRODUKTY_NAZWY[i], shm->wytworzono[i], shm->sprzedano[i]);
    }
    printf("====================================================\n");
}

void sprzatanie(int sig) {
    if (sig == SIGINT) printf("\n[Kierownik] Zamykanie awaryjne...\n");
    shm->sklep_otwarty = 0;
    raport_koncowy();
    for(int i = 0; i < p_count; i++) kill(pids[i], SIGTERM);
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    msgctl(msgid, IPC_RMID, NULL);
    exit(0);
}

int main() {
    signal(SIGINT, sprzatanie);
    key_t key = ftok(".", PROJEKT_ID);
    shmid = shmget(key, sizeof(shared_data_t), IPC_CREAT | 0600);
    shm = (shared_data_t*)shmat(shmid, NULL, 0);
    memset(shm, 0, sizeof(shared_data_t));
    shm->sklep_otwarty = 1;

    semid = semget(key, 2, IPC_CREAT | 0600);
    semctl(semid, 0, SETVAL, MAX_KLIENCI_W_SRODKU);
    semctl(semid, 1, SETVAL, 1);
    msgid = msgget(key, IPC_CREAT | 0600);

    pid_t p;
    p = fork(); if(p==0) { execl("./piekarz", "piekarz", NULL); exit(0); } pids[p_count++] = p;
    p = fork(); if(p==0) { execl("./kasjer", "kasjer", "1", NULL); exit(0); } pids[p_count++] = p;

    int drugi_kasjer = 0;
    struct msqid_ds q_stat;

    for (int i = 0; i < 20; i++) {
        usleep(600000); // Nowy klient co 0.6s
        
        // Monitoring kolejki
        msgctl(msgid, IPC_STAT, &q_stat);
        if (q_stat.msg_qnum > 3 && !drugi_kasjer) {
            printf("\n[Kierownik] Duża kolejka (%ld osób)! Otwieram drugą kasę.\n", q_stat.msg_qnum);
            p = fork();
            if(p==0) { execl("./kasjer", "kasjer", "2", NULL); exit(0); }
            pids[p_count++] = p;
            drugi_kasjer = 1;
        }

        p = fork();
        if (p == 0) {
            char id_s[10]; sprintf(id_s, "%d", i);
            execl("./klient", "klient", id_s, NULL);
            exit(0);
        }
        pids[p_count++] = p;
    }

    sleep(15);
    sprzatanie(0);
    return 0;
}