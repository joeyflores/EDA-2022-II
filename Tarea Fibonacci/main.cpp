#include <iostream>

using namespace std;

int fib(int n){
  if(n<=1) return n;
  return fib(n-1)+fib(n-2);
}

int fibonacci(int n){
  int Fib[n+1];
  Fib[0]=0;
  Fib[1]=1;

  for(int i=2;i<=n;i++){
    Fib[i]=Fib[i-1]+Fib[i-2];
  }
  
  return Fib[n];
}

void multi(int F[2][2], int M[2][2]) {
   int x = F[0][0] * M[0][0] + F[0][1] * M[1][0];
   int y= F[0][0] * M[0][1] + F[0][1] * M[1][1];
   int z = F[1][0] * M[0][0] + F[1][1] * M[1][0];
   int w = F[1][0] * M[0][1] + F[1][1] * M[1][1];
   F[0][0] = x;
   F[0][1] = y;
   F[1][0] = z;
   F[1][1] = w;

  /*cout<<"MATRIZ F: "<<endl;
  for(int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      cout<<F[i][j]<<" ";
    }
    cout<<endl;
  }
  cout<<"MATRIZ M: "<<endl;
  for(int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      cout<<M[i][j]<<" ";
    }
    cout<<endl;
  }
  cout<<endl;*/
}


void expo(int F[2][2], int n) {
   if (n == 0 || n == 1)return;
   int M[2][2] = {{1,1},{1,0}};
   expo(F, n / 2);
   multi(F, F);
   if (n % 2 != 0) multi(F, M);
}
int fibo_matrix(int n) {
   int F[2][2] = {{1,1},{1,0}};
   if (n == 0) return 0;
   expo(F, n - 1);
   return F[0][0];
}

int main() {
  cout<<"FIBONACCI RECURSIVO: "<<fib(6)<<endl;
  cout<<"FIBONACCI ITERATIVO: "<<fibonacci(6)<<endl;

  cout<<"FIBONACCI MATRIX: "<<fibo_matrix(6)<<endl;
}