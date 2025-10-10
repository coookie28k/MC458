#include <stdio.h>
#include <iostream>
using namespace std;

int mdc(int a, int b) {
    if (b == 0) return a;
    return mdc(b, a % b);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int N, F1, F2;
    cin >> N;
    while (N--) {
        cin >> F1 >> F2;
        cout << mdc(F1, F2) << endl;
    }
    return 0;
}
