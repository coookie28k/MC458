#include <stdio.h>
#include <iostream>
using namespace std;
int calls;

int fib(int x){
    int resultado;
    if (x==0) {
        calls++;
        return 0;
    }
    if (x==1){
        calls++;
        return 1;
    }
    else {
       calls++;
        resultado = fib(x-1) + fib(x-2);
        return resultado;
    }
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int N;
    cin >> N;
    while(N--){
        int X;
        calls = -1; //porque ele vai chamar pra fib(X) e ai nao conta
        cin >> X;
        int resultado_fib = fib(X);
        cout << "fib(" << X << ") = " << calls << " calls = " << resultado_fib << endl;
    }

    return 0;
}
