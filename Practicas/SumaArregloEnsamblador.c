#include <stdio.h>

#define TAMANO 5

int main(void) {

    int arreglo[TAMANO] = {10, 20, 30, 40, 50};
    int suma = 0;

    __asm__(
        "movq $0, %%rcx\n\t"
        "movl $0, %%eax\n\t"

        "mi_bucle:\n\t"
        "cmpq %2, %%rcx\n\t"
        "jge fin_bucle\n\t"

        "addl (%1, %%rcx, 4), %%eax\n\t"

        "incq %%rcx\n\t"
        "jmp mi_bucle\n\t"

        "fin_bucle:\n\t"
        "movl %%eax, %0\n\t"

        : "=r"(suma)
        : "r"(arreglo), "r"((long)TAMANO) /* Casteo explícito a 64 bits */
        : "%eax", "%rcx", "memory"
    );

    printf("Suma = %d\n", suma);

    return 0;
}
