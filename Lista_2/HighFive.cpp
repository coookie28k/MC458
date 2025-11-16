#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

ll Intercala(vector<int>& A, int p, int q, int r, vector<int>& B) {
    for (int i = p; i <= q; i++) B[i] = A[i];           
    for (int j = q + 1; j <= r; j++) B[r + q + 1 - j] = A[j]; 
    int i = p;
    int j = r;
    ll cont = 0;
    for (int k = p; k <= r; k++) {
        if (B[i] <= B[j]) {
            A[k] = B[i];
            i++;
        } else {
            A[k] = B[j];
            j--;
            cont += (q - i + 1); 
        }
    }
    return cont;
}

ll MergeSortHF(vector<int>& A, int p, int r, vector<int>& B) {
    if (p >= r) return 0;
    int q = (p + r) / 2;
    ll esq = MergeSortHF(A, p, q, B);
    ll dir = MergeSortHF(A, q + 1, r, B);
    ll entre = Intercala(A, p, q, r, B);
    return esq + dir + entre;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int N;
    while (cin >> N) {
        vector<int> fila(N);
        for (int i = N - 1; i >= 0; i--) cin >> fila[i]; 
        vector<int> B(N);
        ll HF = MergeSortHF(fila, 0, N - 1, B);
        cout << HF << "\n";
    }
    return 0;
}
