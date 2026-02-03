#include "ciastkarnia.h"

int k_typy[10], k_ile[10], w_k = 0;
int s_id;
shared_data_t *sh_ptr;

void ewakuacja(int sig) {
    if (w_k > 0) {
        sem_op(s_id, 1, -1);
        for(int i=0; i<w_k; i++) sh_ptr->porzucono[k_typy[i]] += k_ile[i];
        sem_op(s_id, 1, 1);
    }
    sem_op(s_id, 0, 1);
    exit(0);
}

int main() {
    srand(time(NULL) ^ getpid());
    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    int sh_id = shmget(k, sizeof(shared_data_t), 0666);
    s_id = semget(k, 2, 0666);
    int mq = msgget(k, 0666);
    sh_ptr = shmat(sh_id, NULL, 0);

    signal(SIGUSR2, ewakuacja);

    if (sh_ptr->sklep_otwarty && sem_op(s_id, 0, -1) == 0) {
        usleep(500000 + rand()%1000000); 

        sem_op(s_id, 1, -1);
        for(int i=0; i<3; i++) {
            int t = rand() % P_TYPY;
            if (sh_ptr->stan_podajnika[t] > 0) {
                sh_ptr->stan_podajnika[t]--;
                k_typy[w_k] = t; k_ile[w_k] = 1; w_k++;
            }
        }
        if (w_k < 2) { // Zwrot jesli za malo
            for(int i=0; i<w_k; i++) sh_ptr->stan_podajnika[k_typy[i]]++;
            w_k = 0;
        }
        sem_op(s_id, 1, 1);

        if (w_k >= 2) {
            order_t o; memset(&o, 0, sizeof(order_t));
            o.mtype = 1; o.klient_pid = getpid(); o.liczba_pozycji = w_k;
            memcpy(o.typy, k_typy, sizeof(int)*10);
            memcpy(o.ilosci, k_ile, sizeof(int)*10);
            msgsnd(mq, &o, MSG_SIZE, 0);
            printf("[Klient %d] Kupil %d szt. i idzie do kasy.\n", getpid(), w_k);
        } else {
            printf("[Klient %d] Brak towaru, wychodzi.\n", getpid());
        }
        fflush(stdout);
        sem_op(s_id, 0, 1);
    }
    shmdt(sh_ptr);
    return 0;
}