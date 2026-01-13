#include "../src/include/ciastkarnia.h"
#include <stdio.h>

int main(void) {
    shared_data_t shm = {0};

    for (int i = 0; i < 10000; i++)
        shm.klienci_w_kolejce++;

    if (shm.klienci_w_kolejce != 10000) {
        printf("FAIL: błąd licznika kolejki\n");
        return 1;
    }

    printf("OK: duża kolejka obsłużona\n");
    return 0;
}
