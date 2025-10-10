#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;
vector<vector<int>> grid;
vector<vector<bool>> visitados;

int dx[4] = {-1, 1, 0, 0}; // cima, baixo, esquerda, direita
int dy[4] = {0, 0, -1, 1};

bool PoliciaeLadrao(int x, int y){

    visitados[x][y] = true;
    if ((x == 4) && (y == 4)) return true;
    for (int k = 0; k < 4; k++){
        int novo_x = x + dx[k];
        int novo_y = y + dy[k];
        if(novo_x >= 0 && novo_x < 5 && novo_y >= 0 && novo_y < 5){ //ta dentro do grid
            if (!visitados[novo_x][novo_y] && grid[novo_x][novo_y] == 0){ //nao foi visitado e pode ir no quadrado
                if (PoliciaeLadrao(novo_x, novo_y)) return true; //se o caminho der nos ladroes
            }
        }
    }
    return false;
    
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    int T;
    cin >> T;
    while(T--){
        grid.assign(5, vector<int>(5, 0));
        visitados.assign(5, vector<bool>(5, false));
        for (int i = 0; i < 5; i++){
            for (int j = 0; j < 5; j++){
                int elemento;
                cin >> elemento;
                grid[i][j] = elemento;
            }
        }
        if (grid[0][0] == 1 || grid[4][4] == 1) cout << "ROBBERS" << endl;
        else if (PoliciaeLadrao(0,0)) cout << "COPS" << endl;
        else cout << "ROBBERS" << endl;
    }
    return 0;
}
