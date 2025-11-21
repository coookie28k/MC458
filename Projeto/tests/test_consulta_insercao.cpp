#include "../estrutura_um.h" // Hash
#include "../estrutura_dois.h" // Tree
#include "util_medicao.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <random>

using namespace std;

mt19937 rng(42); 

// Alterado para formato CSV
void imprimir_csv(string op, string estrutura, long long qtd_ops, long long tempo, long long mem) {
    cout << op << "," 
         << estrutura << "," 
         << qtd_ops << "," 
         << tempo << "," 
         << mem << endl;
}

void teste_insercao(long long qtd_ops) {
    int dim_gigante = 1000000; 
    
    vector<int> is(qtd_ops);
    vector<int> js(qtd_ops);
    vector<double> vals(qtd_ops);
    
    uniform_int_distribution<int> dist_coord(0, dim_gigante - 1);
    uniform_real_distribution<double> dist_val(1.0, 100.0);

    for(int k=0; k<qtd_ops; k++) {
        is[k] = dist_coord(rng);
        js[k] = dist_coord(rng);
        vals[k] = dist_val(rng);
    }

    Cronometro cron;
    long long t_e1 = 0, t_e2 = 0;
    long long m_e1 = 0, m_e2 = 0;

    // --- Estrutura 1 (Hash) ---
    {
        start_tracking(); 
        MatrizEsparsaHashDup A(dim_gigante, dim_gigante);
        
        cron.comecar();
        for(int k=0; k<qtd_ops; k++) {
            A.set(is[k], js[k], vals[k]);
        }
        t_e1 = cron.fim_ns();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    // --- Estrutura 2 (Tree) ---
    {
        start_tracking();
        MatrizEsparsaTreeDup A(dim_gigante, dim_gigante);
        
        cron.comecar();
        for(int k=0; k<qtd_ops; k++) {
            A.set(is[k], js[k], vals[k]);
        }
        t_e2 = cron.fim_ns();
        m_e2 = get_tracked_bytes();
        stop_tracking();
    }

    imprimir_csv("SET", "Hash", qtd_ops, t_e1, m_e1);
    imprimir_csv("SET", "Tree", qtd_ops, t_e2, m_e2);
}

void teste_consulta(long long qtd_ops) {
    int dim_gigante = 1000000;
    
    vector<int> is(qtd_ops);
    vector<int> js(qtd_ops);
    uniform_int_distribution<int> dist_coord(0, dim_gigante - 1);
    
    MatrizEsparsaHashDup A_Hash(dim_gigante, dim_gigante);
    MatrizEsparsaTreeDup A_Tree(dim_gigante, dim_gigante);

    for(int k=0; k<qtd_ops; k++) {
        int i = dist_coord(rng);
        int j = dist_coord(rng);
        is[k] = i; 
        js[k] = j;
        A_Hash.set(i, j, 1.5);
        A_Tree.set(i, j, 1.5);
    }

    // Misturar acessos
    for(int k=0; k<qtd_ops; k++) {
        if (k % 2 != 0) {
            is[k] = dist_coord(rng); 
            js[k] = dist_coord(rng);
        }
    }

    Cronometro cron;
    long long t_e1 = 0, t_e2 = 0;
    volatile double dummy = 0;

    // --- Hash ---
    {
        start_tracking();
        cron.comecar();
        for(int k=0; k<qtd_ops; k++) {
            dummy = A_Hash.getElemento(is[k], js[k]);
        }
        t_e1 = cron.fim_ns();
        stop_tracking();
    }

    // --- Tree ---
    {
        start_tracking();
        cron.comecar();
        for(int k=0; k<qtd_ops; k++) {
             dummy = A_Tree.getElemento(is[k], js[k]);
        }
        t_e2 = cron.fim_ns();
        stop_tracking();
    }

    imprimir_csv("GET", "Hash", qtd_ops, t_e1, 0);
    imprimir_csv("GET", "Tree", qtd_ops, t_e2, 0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    // IMPRIMIR CABEÇALHO CSV
    cout << "Operacao,Estrutura,Ops,Tempo_ns,Memoria_Bytes" << endl;

    // Aumentei um pouco a carga para o gráfico ficar bonito
    long long quantidades[] = {1000, 10000, 50000, 100000, 500000, 1000000};

    for (long long q : quantidades) {
        teste_insercao(q);
    }

    for (long long q : quantidades) {
        teste_consulta(q);
    }

    return 0;
}