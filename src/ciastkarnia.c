#include "ciastkarnia.h"


void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int sem_op(int semid, int sem_num, int op) {
    struct sembuf sb = {
        .sem_num = sem_num,
        .sem_op  = (short)op,
        .sem_flg = SEM_UNDO
    };

    while (semop(semid, &sb, 1) == -1) {
        if (errno == EINTR) continue;
        return -1;
    }
    return 0;
}
