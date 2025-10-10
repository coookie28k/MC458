#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

// pre = raiz esq dir
//inf = esq raiz dir
// pos = esq dir raiz
string Posfixo(string pre, string inf){
    if (pre.empty()) return "";
    char raiz = pre[0]; 
    int posicao = inf.find(raiz); // antes da raiz temos esq na inf e apos a raiz temos dir na inf
    string esq = Posfixo(pre.substr(1, posicao), inf.substr(0, posicao));
    string dir = Posfixo(pre.substr(posicao+1), inf.substr(posicao+1));
    return esq + dir + raiz;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    string prefixo, infixo;
    while(cin >> prefixo >> infixo){
        cout << Posfixo(prefixo, infixo) << endl;
    }
    return 0;
}
