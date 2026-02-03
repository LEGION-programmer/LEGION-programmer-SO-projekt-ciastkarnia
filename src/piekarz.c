#include "ciastkarnia.h"

int main() {
    srand(time(NULL));
    key_t k = ftok(FTOK_PATH, PROJEKT_ID);
    int sh_id = shmget(k, sizeof(shared_data_t), 0666);
    int s_id = semget(k, 2, 0666);
    shared_data_t *sh = shmat(sh_id, NULL, 0);

    while(1) {
        sleep(2);
        sem_op(s_id, 1, -1);
        int dowieziono = 0;
        for(int i=0; i<P_TYPY; i++) {
            if (sh->stan_podajnika[i] < 20) {
                int ile = 1 + rand()%3;
                sh->stan_podajnika[i] += ile;
                sh->wytworzono[i] += ile;
                dowieziono++;
            }
        }
        sem_op(s_id, 1, 1);
        if (dowieziono) {
            printf("[Piekarz] Dostawa na podajniki.\n");
            fflush(stdout);
        }
    }
    return 0;
}