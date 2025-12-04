#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ciastkarnia.h"

int main(int argc, char *argv[]) {
    int P = 1;      // liczba piekarzy
    int N = 20;     // pojemność magazynu
    int K = 2;      // liczba kasjerów
    int czas = 60;  // czas symulacji

    // TODO: dodać parsowanie argumentów z argv

    printf("Uruchamiam symulację ciastkarni...\n");

    // Inicjalizacja magazynu w pamięci dzielonej
    init_magazyn(N);

    pid_t pid;
    for(int i=0; i<P; i++) {
        pid = fork();
        if(pid < 0) {
            perror("fork piekarz");
            exit(EXIT_FAILURE);
        }
        if(pid == 0) {
            piekarz_proc(i); // proces piekarza
            exit(0);
        }
    }

    // TODO: uruchomić procesy kasjerów i klientów w następnych commitach

    // Czekanie na piekarzy
    for(int i=0; i<P; i++) {
        wait(NULL);
    }

    // Sprzątanie pamięci dzielonej i semaforów
    cleanup_magazyn();

    printf("Symulacja zakończona.\n");
    return 0;
}
