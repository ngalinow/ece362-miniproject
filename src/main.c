#include <stdlib.h>
#include "stm32f0xx.h"

extern void internal_clock();

int main() {
    internal_clock();

    int x = 4;
    int y = 5;

    int z = x + y;

    return EXIT_SUCCESS;
}