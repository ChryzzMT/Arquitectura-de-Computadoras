#include <stdio.h>
#include <stdint.h>

typedef union{
  unsigned char byte;
  struct{
    unsigned char b0: 1;
    unsigned char b1: 1;
    unsigned char b2: 1;
    unsigned char b3: 1;
    unsigned char b4: 1;
    unsigned char b5: 1;
    unsigned char b6: 1;
    unsigned char b7: 1;
  }bits;
}Reg8Bits;

int main(){
  int8_t a=120;
  int8_t b=20;
  int8_t resultado=a+b;
  printf("%d\n",resultado);
  if(a>0 && b>0){
    if (a>INT8_MAX-b){
    printf("Desborde\n");
    }
  }
  if(a<0 && b<0){
    if(a<INT8_MIN+b){
      printf("Desborde\n");
    }
  }
  Reg8Bits miDato;
  miDato.byte= resultado;
  printf("%d%d%d%d%d%d%d%d",
  miDato.bits.b7,miDato.bits.b6,
  miDato.bits.b5,miDato.bits.b4,
  miDato.bits.b3,miDato.bits.b2,
  miDato.bits.b1,miDato.bits.b0);
}
