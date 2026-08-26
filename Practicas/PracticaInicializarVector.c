#include<stdio.h>
#define TAMANO 5
int main(void){
    int arreglo[TAMANO]={0};
    __asm__(
    "movl $0, %%ecx\n\t"
    "mi_bucle:\n\t"
    "cmpl %1, %%ecx\n\t"
    "jge fin_bucle\n\t"
    "movl %%ecx, %%eax\n\t"
    "addl $1, %%eax\n\t"
    "imull $10, %%eax\n\t"
    "movl %%eax,(%0, %%rcx,4)\n\t"
    "incl %%ecx\n\t"
    "jmp mi_bucle\n\t"
    "fin_bucle:\n\t"
    :
    :"r"(arreglo), "r"(TAMANO)
    :"%rax","%rcx","memory"
    );
    
    printf("COntenido del arreglo\n");
    for(int i=0;i<TAMANO;i++){
      printf("arreglo[%d] = %d\n", i,arreglo[i]);
    }
    return 0;
}
