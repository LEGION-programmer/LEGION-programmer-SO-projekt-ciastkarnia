#include "ciastkarnia.h"

volatile sig_atomic_t stop = 0;
void h_stop(int s) { stop = 1; }

int main(int argc, char *argv[]) {
    int id = (argc > 1) ? atoi(argv[1]) : 1;
    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    int sh_id = shmget(k, sizeof(shared_data_t), 0666);
    int s_id = semget(k, 2, 0666);
    int mq = msgget(k, 0666);
    shared_data_t *sh = shmat(sh_id, NULL, 0);
    order_t z;

    signal(SIGUSR1, h_stop);
    printf("[Kasjer %d] Stanowisko otwarte.\n", id);
    fflush(stdout);

    while(1) {
        if (msgrcv(mq, &z, MSG_SIZE, 1, stop ? IPC_NOWAIT : 0) != -1) {
            printf(GREEN "[Kasjer %d] Obsluguje klienta %d\n" RESET, id, z.klient_pid);
            fflush(stdout);
            sem_op(s_id, 1, -1);
            for(int i=0; i<z.liczba_pozycji; i++) sh->sprzedano[z.typy[i]] += z.ilosci[i];
            sem_op(s_id, 1, 1);
        } else {
            if (stop || errno == ENOMSG) break;
        }
    }
    printf("[Kasjer %d] Stanowisko zamkniete.\n", id);
    fflush(stdout);
    return 0;
}