#include "include/ciastkarnia.h"

// Wymóg 4.1.c: Obsługa błędów z perror
void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Wymóg 5.2.e: Synchronizacja semaforami System V
void sem_op(int semid, int sem_num, int op) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = op;
    sb.sem_flg = 0;
    if (semop(semid, &sb, 1) == -1) {
        // Jeśli nie jest to przerwanie przez sygnał, rzuć błąd
        if (errno != EINTR) handle_error("Błąd semop");
    }
}