#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool sumar_uint8_con_carry(uint8_t a, uint8_t b, uint8_t * resultado){
  unsigned char carry_flag;
  __asm__(
    "addb %[val_b], %[val_a]\n\t"
    "setc %[cf]\n\t"
    : [val_a] "+q"(a),
      [cf] "=q" (carry_flag)    
    : [val_b] "q"(b)
    : "cc"
  );                 

  *resultado = a;
  return (bool) carry_flag;
}                       

int main(){
  uint8_t res;

  bool hay_carry = sumar_uint8_con_carry(200, 50, &res); 
  printf("200+50 = %u | Carry Flag: %s\n", res, hay_carry ? "SI (Acarreo)" : "NO");

  hay_carry = sumar_uint8_con_carry(200, 60, &res);
  printf("200+60 = %u | Carry Flag: %s\n", res, hay_carry ? "SI (Acarreo)" : "NO"); 

  return 0;
}
