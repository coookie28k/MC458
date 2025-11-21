#include "../densa.h"
#include "../estrutura_um.h" // Hash
#include "../estrutura_dois.h" // Tree
#include "../gerador.h"
#include "util_medicao.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

// Função auxiliar para saída CSV
void imprimir_csv(string estrutura, int n, double esp, long long tempo, long long mem) {
    cout << "CONSTRUCAO," 
         << estrutura << "," 
         << n << "," 
         << esp << "," 
         << tempo << "," 
         << mem << endl;
}

void teste_construcao(int dimensao, double esparsidade) {
    // Gera a base de dados (mapa) para popular as matrizes
    // Nota: O tempo de geração dessa base NÃO entra na conta, apenas a construção da matriz alvo
    unordered_map<long long, Entry> base = gerar_matriz_esparsa(dimensao, esparsidade);

    Cronometro cron;
    long long t_densa = -1, t_e1 = 0, t_e2 = 0;
    long long m_densa = 0, m_e1 = 0, m_e2 = 0;

    // ======================
    // Teste Matriz Densa
    // ======================
    // Limitamos Densa a 10.000 pois 100.000^2 doubles = ~80 GB de RAM
    if (dimensao <= 10000) { 
        start_tracking();
        cron.comecar();
        {
            MatrizDensa A(dimensao, dimensao);
            for (auto &p : base)
                A.set(p.second.i, p.second.j, p.second.valor);
            
            t_densa = cron.fim_ns();
        }
        stop_tracking();
        m_densa = get_tracked_bytes();
    }

    // ======================
    // Teste Estrutura 1 (Hash)
    // ======================
    {
        start_tracking();
        cron.comecar();
        
        // Escopo interno para garantir medição precisa da construção
        {
            MatrizEsparsaHashDup B(dimensao, dimensao);
            for (auto &p : base)
                B.set(p.second.i, p.second.j, p.second.valor);
            
            t_e1 = cron.fim_ns();
        } // B é destruída aqui, mas o tracking parou antes se colocarmos stop dentro?
          // MELHOR: Medir tempo até o fim da inserção, parar tracking, pegar memória.
        
        stop_tracking();
        m_e1 = get_tracked_bytes();
    }

    // ======================
    // Teste Estrutura 2 (Tree)
    // ======================
    {
        start_tracking();
        cron.comecar();
        {
            MatrizEsparsaTreeDup C(dimensao, dimensao);
            for (auto &p : base)
                C.set(p.second.i, p.second.j, p.second.valor);
            
            t_e2 = cron.fim_ns();
        }
        stop_tracking();
        m_e2 = get_tracked_bytes();
    }

    // ======================
    // Saída CSV
    // ======================
    imprimir_csv("Densa", dimensao, esparsidade, t_densa, m_densa);
    imprimir_csv("Est1(Hash)", dimensao, esparsidade, t_e1, m_e1);
    imprimir_csv("Est2(Tree)", dimensao, esparsidade, t_e2, m_e2);
}

void teste_todas_construcoes() {
    srand(time(NULL));

    for (int i = 2; i <= 8; i++) {
        double esparsidades[4];
        long long dimensao = pow(10, i);

        // Define esparsidades
        if (i < 4) {
            esparsidades[0] = 0.01; esparsidades[1] = 0.05;
            esparsidades[2] = 0.10; esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }

        for (double e : esparsidades) {
            if (e <= 0.0) continue; // Evita divisão por zero ou erros
            teste_construcao(dimensao, e);
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    // Cabeçalho do CSV
    cout << "Operacao,Estrutura,N,Esparsidade,Tempo_ns,Memoria_Bytes" << endl;

    teste_todas_construcoes();

    return 0;
}