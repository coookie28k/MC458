#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct No {
    int chave;
    No* esq = nullptr;
    No* dir = nullptr;
};

No* I(No* raiz, int n) {
    if (!raiz) {
        No* novo = new No;
        novo->chave = n;
        return novo;   
    }
    if (n < raiz->chave) raiz->esq = I(raiz->esq, n);
    else if (n > raiz->chave) raiz->dir = I(raiz->dir, n);
    return raiz;
}

bool P(No* raiz, int n) {
    if (raiz == NULL) return false;
    else if (n == raiz->chave) return true;
    if (n < raiz->chave) return P(raiz->esq, n);
    else return P(raiz->dir, n);
}

No* maiorVSub(No* raiz) {
    while (raiz && raiz->dir)
        raiz = raiz->dir;
    return raiz;
}

No* R(No* raiz, int x) {
    if (!raiz) return nullptr;
    if (x < raiz->chave) raiz->esq = R(raiz->esq, x);
    else if (x > raiz->chave) raiz->dir = R(raiz->dir, x);
    else {
        if (!raiz->esq && !raiz->dir) { //folha
            delete raiz;
            return nullptr;
        } else if (!raiz->esq) {
            No* aux = raiz->dir;
            delete raiz;
            return aux;
        } else if (!raiz->dir) { 
            No* aux = raiz->esq;
            delete raiz;
            return aux;
        } else {
            //menor valor da subar esq ocupa o espaco
            No* ant = maiorVSub(raiz->esq);
            raiz->chave = ant->chave;
            raiz->esq = R(raiz->esq, ant->chave);
        }
    }
    return raiz;
}

//esq raiz dir
void INFIXA(No* raiz, bool& primeiro) {
    if (!raiz) return;
    INFIXA(raiz->esq, primeiro);
    if (!primeiro) cout << " ";
    cout << raiz->chave;
    primeiro = false;
    INFIXA(raiz->dir, primeiro);
}

//raiz esq dir
void PREFIXA(No* raiz, bool& primeiro) {
    if (!raiz) return;
    if (!primeiro) cout << " ";
    cout << raiz->chave;
    primeiro = false;
    PREFIXA(raiz->esq, primeiro);
    PREFIXA(raiz->dir, primeiro);
}

//esq dir raz
void POSFIXA(No* raiz, bool& primeiro) {
    if (!raiz) return;
    POSFIXA(raiz->esq, primeiro);
    POSFIXA(raiz->dir, primeiro);
    if (!primeiro) cout << " ";
    cout << raiz->chave;
    primeiro = false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    No* raiz = nullptr;
    string comando;
    int n;

    while (cin >> comando) {
        if (comando == "I") {
            cin >> n;
            raiz = I(raiz, n);
        } else if (comando == "R") {
            cin >> n;
            raiz = R(raiz, n);
        } else if (comando == "P") {
            cin >> n;
            cout << n << (P(raiz, n) ? " existe" : " nao existe") << "\n";
        } else if (comando == "INFIXA") {
            bool primeiro = true;
            INFIXA(raiz, primeiro);
            cout << "\n";
        } else if (comando == "PREFIXA") {
            bool primeiro = true;
            PREFIXA(raiz, primeiro);
            cout << "\n";
        } else if (comando == "POSFIXA") {
            bool primeiro = true;
            POSFIXA(raiz, primeiro);
            cout << "\n";
        }
    }

    return 0;
}
