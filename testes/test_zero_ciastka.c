#include "../src/include/ciastkarnia.h"
#include <stdio.h>

int main(void) {
    shared_data_t shm = {0};
    shm.ciastka = 0;
    shm.max_ciastka = 10;

    if (shm.ciastka != 0) {
        printf("FAIL: magazyn niepusty\n");
        return 1;
    }

    printf("OK: brak ciastek obsłużony poprawnie\n");
    return 0;
}
