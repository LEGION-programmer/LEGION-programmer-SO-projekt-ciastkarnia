#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ipc.h"

int main(int argc, char *argv[]) {

    int piekarze = 1;
    int klienci_max = 10;
    int kasjerzy = 2;
    int czas_dzialania = 30;

    // ---- PARSOWANIE ARGUMENTÓW ----
    if (argc != 9) {
        printf("Użycie: ./ciastkarnia -P <piekarze> -N <klienci> -K <kasjerzy> -t <czas>\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-P")) piekarze = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-N")) klienci_max = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-K")) kasjerzy = atoi(argv[i+1]);
        if (!strcmp(argv[i], "-t")) czas_dzialania = atoi(argv[i+1]);
    }

    printf("PARAMETRY:\n");
    printf("  Piekarze: %d\n", piekarze);
    printf("  Limit klientów: %d\n", klienci_max);
    printf("  Kasjerzy: %d\n", kasjerzy);
    printf("  Czas działania: %d s\n", czas_dzialania);

    // ---- INICJALIZACJA IPC ----
    int shmid, semid, msgid;
    shm_data_t *shm_ptr;

    if (init_shared_memory(&shmid, &shm_ptr, klienci_max) != 0) {
        fprintf(stderr, "Błąd: init_shared_memory\n");
        return 1;
    }

    if (init_semaphores(&semid) != 0) {
        fprintf(stderr, "Błąd: init_semaphores\n");
        cleanup_ipc(shmid, -1, -1, shm_ptr);
        return 1;
    }

    if (init_message_queue(&msgid) != 0) {
        fprintf(stderr, "Błąd: init_message_queue\n");
        cleanup_ipc(shmid, semid, -1, shm_ptr);
        return 1;
    }

    printf("\nIPC GOTOWE:\n");
    printf("  SHM ID = %d\n", shmid);
    printf("  SEM ID = %d\n", semid);
    printf("  MSG ID = %d\n\n", msgid);

    printf("Symulacja jeszcze nic nie robi — to będzie w kolejnych commitach.\n");
    printf("Ten commit zawiera jedynie infrastrukturę IPC.\n");

    printf("Czekam 3 sekundy...\n");
    sleep(3);

    // ---- SPRZĄTANIE ----
    cleanup_ipc(shmid, semid, msgid, shm_ptr);

    printf("Zasoby wyczyszczone. Koniec programu.\n");
    return 0;
}
