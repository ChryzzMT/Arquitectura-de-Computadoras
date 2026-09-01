#include <stdio.h>
int main(){
  int arreglo[]={12,34,56};
  int i=2,suma=0;
  while(i>=0){
    suma+=arreglo[i];
    i=i-1;
  }
  printf("suma:%d\n",suma);
  return 0;
}
