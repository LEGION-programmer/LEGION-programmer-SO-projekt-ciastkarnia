#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>   // mkdir
#include <errno.h>
#include <string.h>

#define LOG_DIR "logs"

// Prosta funkcja loggera
void log_message(const char *msg) {
    printf("[LOG]: %s\n", msg);
}

int main() {
    struct stat st = {0};

    // Tworzenie katalogu logs jeśli nie istnieje
    if (stat(LOG_DIR, &st) == -1) {
        if (mkdir(LOG_DIR, 0777) != 0) {
            fprintf(stderr, "Nie udało się utworzyć katalogu '%s': %s\n", LOG_DIR, strerror(errno));
            return 1;
        }
    }

    log_message("Ciastkarnia uruchomiona.");
    log_message("Proces główny (kierownik) działa.");

    // Tutaj w przyszłości będzie inicjalizacja pamięci, semaforów itp.

    printf("Symulacja gotowa do uruchomienia kolejnych procesów.\n");

    return 0;
}
