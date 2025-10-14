#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct Time {
    int id = 0;
    int pontos = 0;
    int marcados = 0;
    int levados = 0;
};

bool melhor(Time A, Time B) {
    if (A.pontos != B.pontos)
        return A.pontos > B.pontos;

    double mediaA = (A.levados > 0) ? (double)A.marcados / A.levados : (double)A.marcados;
    double mediaB = (B.levados > 0) ? (double)B.marcados / B.levados : (double)B.marcados;
    if (mediaA != mediaB)
        return mediaA > mediaB;

    if (A.marcados != B.marcados)
        return A.marcados > B.marcados;

    return A.id < B.id;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int n;
    cin >> n;
    int count = 1;
    while (n != 0){
        vector <Time> times(n);
        for (int i = 0; i < n; i++){
            times[i] = {i + 1, 0, 0, 0};
        }

        int jogos = n * (n-1)/2;
        for (int i = 0; i < jogos; i++){
            int x, y, z, w;
            cin >> x >> y >> z >> w;
      
            times[x-1].marcados += y;
            times[x-1].levados += w;
            times[z-1].marcados += w;
            times[z-1].levados += y;
            if (y > w){
                times[x-1].pontos += 2;
                times[z-1].pontos += 1;
            } else {
                times[x-1].pontos += 1;
                times[z-1].pontos += 2;
            }
        }

        sort(times.begin(), times.end(), melhor);

        cout << "Instancia " << count << "\n";
        for (int i = 0; i < n; i++) {
            cout << times[i].id;
            if (i < n - 1) cout << " ";
        }
        cout << "\n";

        count++;
        cin >> n;
        if (n != 0) cout << "\n";
    }
    return 0;
}
