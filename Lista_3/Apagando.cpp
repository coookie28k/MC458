#include <stdio.h>
#include <iostream>
#include <bits/stdc++.h>

using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, D;
    while (cin >> N >> D, N!= 0 || D!=0){
        string numero;
        cin >> numero;

        string premio_max = "";
        int remover = D;

        for (char d : numero) {
            while (!premio_max.empty() && remover > 0 && premio_max.back() < d) {
                premio_max.pop_back();
                remover--;
            }
            premio_max.push_back(d);
        }

        premio_max.resize(N - D);

        cout << premio_max << endl;
    }
    return 0;
}
