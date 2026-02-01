#include "ciastkarnia.h"

void ucieczka(int sig) {
    (void)sig; 
    _exit(0);
}

int main() {
    signal(SIGUSR2, ucieczka);
    signal(SIGINT, SIG_IGN);

    key_t k = ftok(".", PROJEKT_ID);
    int semid = semget(k, 2, 0);
    int msqid = msgget(k, 0);

    srand(getpid());

    // Próba wejścia do sklepu (Semafor 0)
    if (sem_op(semid, 0, -1) != -1) {
        printf("[Klient %d] Wszedl.\n", getpid());
        
        // Czas na zakupy
        usleep(300000); 

        order_t o;
        o.mtype = 1;
        o.typ = rand() % P_TYPY;
        o.ilosc = (rand() % 3) + 1;

        msgsnd(msqid, &o, sizeof(order_t)-sizeof(long), 0);
        
        sem_op(semid, 0, 1); // Wyjście
        printf("[Klient %d] Wyszedl.\n", getpid());
    }

    return 0;
}