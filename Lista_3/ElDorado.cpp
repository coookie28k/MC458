#include <stdio.h>
#include <iostream>
#include <bits/stdc++.h>

using namespace std;
typedef long long ll;

ll s[101][101];
ll a[101];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    while (cin >> n >> k, n != 0 || k !=0 ) {

        //PD
        for (int i = 1; i <= n; i++) cin >> a[i];
        for (int i = 1; i <= n; i++){
            for (int j = 1; j <= k; j++) s[i][j] = 0;
        } 
        for (int i = 1; i <= n; i++) s[i][1] = 1;
        for (int j = 2; j <= k; j++) {
            for (int i = 1; i <= n; i++) {
                ll soma = 0;
                for (int t = 1; t < i; t++) {
                    if (a[t] < a[i]) soma += s[t][j-1];
                }
                s[i][j] = soma;
            }
        }

        //Resolve
        ll soma_k = 0;
        for (int i = 1; i <= n; i++) soma_k += s[i][k];

        cout << soma_k << endl;
    }

    return 0;
}
