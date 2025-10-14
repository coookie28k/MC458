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

No* insert(No* raiz, int n) {
    if (!raiz) {
        No* novo = new No;
        novo->chave = n;
        return novo;   
    }
    if (n < raiz->chave) raiz->esq = insert(raiz->esq, n);
    else if (n > raiz->chave) raiz->dir = insert(raiz->dir, n);
    return raiz;
}


N* inserir(N* raiz, int chave) {
    N* novo;
    if (raiz == NULL) {
    novo = malloc(sizeof(struct no));
    novo->esq = novo->dir = NULL;
    novo->chave = chave;
    return novo;
    }
    if (chave < raiz->chave)
    raiz->esq = inserir(raiz->esq, chave);
    else
    raiz->dir = inserir(raiz->dir, chave);
    return raiz;
}

No* P(No* raiz, int chave) {
    if (raiz == NULL || chave == raiz->chave)
        return raiz;
    if (chave < raiz->chave)
        return P(raiz->esq, chave);
    else
        return P(raiz->dir, chave);
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int n;
    cin >> n;
    return 0;
}
