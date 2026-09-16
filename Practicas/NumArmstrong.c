#include <cmath>
bool narcissistic( int value ){
  int n=value;
int nc=n;
  int cont=0;
  while(nc>0){
    nc=nc/10;
    cont++;
  }
  int newValue=0;
  while(n>0){
    int dig=n%10;
    dig=pow(dig,cont);
    newValue=newValue+dig;
    n=n/10;
  }
  if (newValue==value){
    return true;
  }return false;
}





