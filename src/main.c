#include "ciastkarnia.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>

pid_t dzieci[32];
int dcount = 0;

void sigint_handler(int sig) {
    printf("\n[MAIN] Kończenie symulacji...\n");
    for (int i = 0; i < dcount; i++)
        kill(dzieci[i], SIGTERM);
}

int main() {
    int P = 2, K = 2, C = 20;
    int max_ciasta = 10;

    shared_data_t *shm;
    int shmid, semid, msgid;

    init_shared_memory(&shm, &shmid, max_ciasta);
    semid = init_semaphore();
    msgid = init_queue();

    struct sigaction sa = {0};
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("=== CIASTKARNIA START ===\n");

    for (int i = 0; i < P; i++) {
        pid_t p = fork();
        if (p == 0) proces_piekarz(i, shm, semid);
        dzieci[dcount++] = p;
    }

    for (int i = 0; i < K; i++) {
        pid_t p = fork();
        if (p == 0) proces_kasjer(i, shm, semid, msgid);
        dzieci[dcount++] = p;
    }

    for (int i = 0; i < C; i++) {
        sem_down(semid);
        shm->klienci_przyszli++;
        shm->kolejka++;
        sem_up(semid);

        pid_t p = fork();
        if (p == 0) proces_klient(i, msgid);
        sleep(1);
    }

    while (wait(NULL) > 0);

    printf("\n=== STATYSTYKA ===\n");
    printf("Klienci przyszli: %d\n", shm->klienci_przyszli);
    printf("Klienci obsłużeni: %d\n", shm->klienci_obsluzeni);
    printf("Sprzedane ciastka: %d\n", shm->sprzedane_ciasta);

    cleanup_shared_memory(shmid, shm);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
