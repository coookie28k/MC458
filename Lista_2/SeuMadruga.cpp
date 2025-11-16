#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

double buscabinH(const vector<ll> &C, ll A, ll N, double menor, double maior) {
    if (maior - menor < 1e-7) return menor;  
    double media = (menor + maior) / 2.0;
    double area = 0;
    for (ll i = 0; i < N; i++)
        area += max(0.0, (double)C[i] - media);
    if (area > A) return buscabinH(C, A, N, media, maior); 
    else return buscabinH(C, A, N, menor, media); 
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    ll N, A ;
    while(cin >> N >> A){  
        if(N == 0 && A == 0) break;
        ll soma = 0;
        vector<ll> comprimento(N);
        ll maximo = 0;
        for (ll i = 0; i < N; i++){
            cin >> comprimento[i];
            soma += comprimento[i];
            if (comprimento[i] > maximo) maximo = comprimento[i];
        }
        if (A == 0 || A == soma) cout << ":D\n";
        else if (A > soma) cout << "-.-\n";
        else {
            double H = buscabinH(comprimento, A, N, 0, maximo);
            cout << fixed << setprecision(4) << H << "\n";
        }
    }
    return 0;
}
