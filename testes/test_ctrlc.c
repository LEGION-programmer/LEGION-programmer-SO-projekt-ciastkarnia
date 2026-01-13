#include "../src/include/ciastkarnia.h"
#include <stdio.h>

int main(void) {
    running = 1;
    sigint_main(SIGINT);

    if (running != 0) {
        printf("FAIL: SIGINT nie zatrzymał programu\n");
        return 1;
    }

    printf("OK: SIGINT działa poprawnie\n");
    return 0;
}
