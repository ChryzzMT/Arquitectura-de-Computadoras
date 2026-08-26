#include <stdio.h>
#include <stdbool.h>
bool restar_y_detectar_zero(int a, int b, int *resultado){
  unsigned char zero_flag;
  __asm__(
  "subl %[val_b], %[val_a]\n\t"
  "setz %[zf] \n\t"
  :[val_a] "+r"(a),
  [zf] "=q" (zero_flag)
  :[val_b] "r"(b)
  :"cc"
  );
  *resultado=a;
  return (bool)zero_flag;
}

int main(){
  int res;
  bool es_cero=restar_y_detectar_zero(42,42,&res);
  printf("42-42 =%d|Zero Flag activa: %s\n", res,es_cero ? "SI" : "NO");
  
  es_cero = restar_y_detectar_zero(50,20,&res);
  printf("50-20 =%d | Zero Flag activa: %s\n", res, es_cero? "SI": "NO");
  
  return 0;
}
