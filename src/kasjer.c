#include "ciastkarnia.h"

int main() {
    key_t klucz = ftok(".", PROJECT_ID);
    int msgid = msgget(klucz, 0666);
    int semid = semget(klucz, 5, 0666);
    int shmid = shmget(klucz, sizeof(shared_data_t), 0666);
    shared_data_t *data = shmat(shmid, NULL, 0);

    msg_t zam;
    while(1) {
        if (msgrcv(msgid, &zam, sizeof(msg_t) - sizeof(long), 1, 0) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        sleep(1); 
        sem_op(semid, 0, -1); // MUTEX chroniący shared memory przed wyścigiem kasjerów
        data->sprzedane_ciastka += zam.laczna_liczba_ciastek;
        sem_op(semid, 0, 1);
    }
    return 0;
}