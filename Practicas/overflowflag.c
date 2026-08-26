#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool sumar_int8_con_overflow(int8_t a, int8_t b, int8_t *resultado){ 
  unsigned char overflow_flag;                      
  __asm__(
    "addb %[val_b], %[val_a]\n\t"
    "seto %[of]\n\t"                                   
    : [val_a] "+q" (a),
      [of] "=q" (overflow_flag)
    : [val_b] "q" (b)
    : "cc"
  );
  *resultado = a;                                      
  return (bool)overflow_flag;
}

int main(){
  int8_t res;
  
  bool hay_overflow = sumar_int8_con_overflow(100, 20, &res);
  printf("100+20 = %d | Overflow Flag: %s\n", res, hay_overflow ? "SI" : "NO");

  hay_overflow = sumar_int8_con_overflow(100, 30, &res);
  printf("100+30 = %d | Overflow Flag: %s\n", res, hay_overflow ? "SI (Error de signo)" : "NO"); // Corregido: ';' al final

  return 0;
}    

