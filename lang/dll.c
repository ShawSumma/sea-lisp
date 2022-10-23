
#include <stdint.h>

int32_t vm_add(int32_t a, int32_t b) {
    return a + b;
}

void vm_print(int32_t a) {
    printf("%i\n", (int) a);
}
