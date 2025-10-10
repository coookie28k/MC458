#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;

int N, M;
vector<vector<int>> grid;

// para oa vizinhos
int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

int pintando(int x, int y){
    if (x < 0 || x > N-1 || y < 0 || y > M-1) return 0;
    if (grid[x][y] != 0) return 0;

    grid[x][y] = 2; // quadrado pintado é 2
    int count = 1;

    for (int k = 0; k < 8; k++) {
        count += pintando(x + dx[k], y + dy[k]);
    }

    return count;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int X, Y, K;
    cin >> N >> M >> X >> Y >> K;
    grid.assign(N, vector<int>(M, 0)); //deixo os quadrados vazios como 0
    while(K--){
        int A, B;
        cin >> A >> B;
        grid[A-1][B-1] = 1; //deixo o squadrados cheios como 1
    }
    int resultado = pintando(X-1, Y-1);
    cout << resultado << '\n';
    return 0;
}

