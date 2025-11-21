#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

long long gerarChave(long long linha, long long coluna, long long n) {
    return linha * n + coluna; // linearizacao da chave (i, j)
}

struct Entry {
    int i, j, valor;
};

unordered_map<long long, Entry>
gerar_matriz_esparsa(long long dimensao, double esparsidade)
{
    unordered_map<long long, Entry> matriz;

    long long total = dimensao * dimensao;
    long long num_elementos = llround(total * esparsidade);

    if (num_elementos == 0) return matriz;

    for (long long k = 0; k < num_elementos; k++) {
        bool posicao_valida = false;

        while (posicao_valida == false) {
            long long linha  = rand() % dimensao;
            long long coluna = rand() % dimensao;

            long long chave = gerarChave(linha, coluna, dimensao);

            // Verifica se a chave ja foi usada, ou seja, se a posicao ja foi preenchida
            if (matriz.find(chave) == matriz.end()) {
                posicao_valida = true;
                int valor = (rand() % 100) + 1;

                Entry e;
                e.i = linha;
                e.j = coluna;
                e.valor = valor;
                matriz[chave] = e;
            }
        }
    }

    return matriz;
}

void gerarTodasMatrizesEsparsas() {
    srand(time(NULL));

    for (int i = 1; i <= 3; i++) {
        double esparsidades[4];

        long long dimensao = pow(10, i);

        if (i < 4) {
            esparsidades[0] = 0.01;
            esparsidades[1] = 0.05;
            esparsidades[2] = 0.10;
            esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }

        for (double e : esparsidades) {
            if (e == 0.0) continue; // ignora elemento vazio do vetor, para i >= 4

            auto matriz = gerar_matriz_esparsa(dimensao, e);
            cout << "Dim: " << dimensao << "x" << dimensao
                 << "  esparsidade=" << e
                 << "  n_elementos=" << matriz.size() << endl;
        }
    }
}
