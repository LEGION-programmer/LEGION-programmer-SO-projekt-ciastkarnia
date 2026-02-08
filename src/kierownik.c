#include "ciastkarnia.h"
#include <sys/wait.h>

int m_id = -1, s_id = -1, sh_id = -1;
pid_t p_pid = 0, k1_pid = 0, k2_pid = 0;

void sprzatacz(int sig) {
    (void)sig;
    signal(SIGINT, SIG_IGN);
    shared_data_t *dane = shmat(sh_id, NULL, 0);
    
    printf("\n%s==========================================%s\n", YELLOW, RESET);
    printf("%s       RAPORT KOŃCOWY PIEKARNI          %s\n", BOLD, RESET);
    printf("%s==========================================%s\n", YELLOW, RESET);
    if (dane != (void*)-1) {
        printf(" Obsłużeni klienci:      %d / 5000\n", dane->klienci_lacznie);
        printf(" Sprzedane ciastka:      %d\n", dane->sprzedane_ciastka);
        printf(" Porzucone zamówienia:   %d\n", dane->porzucone_zamowienia);
        printf(" Wyprodukowane łącznie:  %d\n", dane->wyprodukowane_lacznie);
        if (dane->klienci_lacznie > 0)
            printf(" Średnia ciastek/klient: %.2f\n", (float)dane->sprzedane_ciastka / dane->klienci_lacznie);
    }
    
    msgctl(m_id, IPC_RMID, NULL);
    semctl(s_id, 0, IPC_RMID);
    shmctl(sh_id, IPC_RMID, NULL);
    
    kill(0, SIGKILL);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) exit(1);
    int max_poj = atoi(argv[1]);
    int prog_kasy = atoi(argv[2]);

    signal(SIGINT, sprzatacz);
    signal(SIGTERM, sprzatacz);

    key_t klucz = ftok(".", PROJECT_ID);
    m_id = msgget(klucz, IPC_CREAT | 0666);
    s_id = semget(klucz, 5, IPC_CREAT | 0666);
    sh_id = shmget(klucz, sizeof(shared_data_t), IPC_CREAT | 0666);
    
    struct msqid_ds q_ds;
    msgctl(m_id, IPC_STAT, &q_ds);
    q_ds.msg_qbytes = 65536; 
    msgctl(m_id, IPC_SET, &q_ds);

    semctl(s_id, 0, SETVAL, 1);       
    semctl(s_id, 1, SETVAL, max_poj); 

    shared_data_t *dane = shmat(sh_id, NULL, 0);
    memset(dane, 0, sizeof(shared_data_t));

    if((p_pid = fork()) == 0) execl("./piekarz", "./piekarz", NULL);
    if((k1_pid = fork()) == 0) execl("./kasjer", "./kasjer", "1", NULL);

    sleep(3);

    int puste_cykle = 0;
    while(1) {
        struct msqid_ds buf;
        msgctl(m_id, IPC_STAT, &buf);
        int q = buf.msg_qnum;
        int w_srodku = max_poj - semctl(s_id, 1, GETVAL);

        if (q > prog_kasy && k2_pid == 0) {
            if((k2_pid = fork()) == 0) execl("./kasjer", "./kasjer", "2", NULL);
        } else if (q <= (prog_kasy/2) && k2_pid != 0) {
            kill(k2_pid, SIGTERM);
            waitpid(k2_pid, NULL, 0);
            k2_pid = 0;
        }

        printf("\r[Kierownik] Sklep: %d/%d | Kolejka kasy: %d | Sprzedane: %d", 
               w_srodku, max_poj, q, dane->sprzedane_ciastka);
        fflush(stdout);

        if (w_srodku == 0 && q == 0 && dane->klienci_lacznie >= 5000) {
            puste_cykle++;
            if (puste_cykle > 30) sprzatacz(SIGINT);
        } else {
            puste_cykle = 0;
        }
        usleep(200000);
    }
    return 0;
}