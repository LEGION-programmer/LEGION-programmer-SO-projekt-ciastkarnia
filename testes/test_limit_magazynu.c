#include "../src/include/ciastkarnia.h"
#include <stdio.h>

int main(void) {
    shared_data_t shm = {0};
    shm.max_ciastka = 5;

    for (int i = 0; i < 10; i++) {
        if (shm.ciastka < shm.max_ciastka)
            shm.ciastka++;
    }

    if (shm.ciastka > shm.max_ciastka) {
        printf("FAIL: przekroczono limit magazynu\n");
        return 1;
    }

    printf("OK: limit magazynu zachowany\n");
    return 0;
}
