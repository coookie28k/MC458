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

// Função ajustada para saída CSV
void imprimir_csv(string op, string estrutura, int n, double esp, long long tempo, long long mem) {
    cout << op << "," 
         << estrutura << "," 
         << n << "," 
         << esp << "," 
         << tempo << "," 
         << mem << endl;
}

// ==========================================
// TESTE DA TRANSPOSTA
// ==========================================
void teste_transposta(int dim, double esp) {
    auto base = gerar_matriz_esparsa(dim, esp);

    Cronometro cron;
    long long t_densa = -1, t_e1 = 0, t_e2 = 0;
    long long m_densa = 0, m_e1 = 0, m_e2 = 0;

    // --- Densa ---
    if (dim <= 10000) { 
        start_tracking();
        {
            MatrizDensa A(dim, dim);
            for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking(); 
            cron.comecar();
            MatrizDensa T = A.transposta();
            t_densa = cron.fim_ns();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    // --- Estrutura 1 (Hash) ---
    {
        MatrizEsparsaHashDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.transpor(); 
        t_e1 = cron.fim_ns();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    // --- Estrutura 2 (Tree) ---
    {
        MatrizEsparsaTreeDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.transpor();
        t_e2 = cron.fim_ns();
        m_e2 = get_tracked_bytes();
        stop_tracking();
    }

    imprimir_csv("TRANS", "Densa", dim, esp, t_densa, m_densa);
    imprimir_csv("TRANS", "Est1(Hash)", dim, esp, t_e1, m_e1);
    imprimir_csv("TRANS", "Est2(Tree)", dim, esp, t_e2, m_e2);
}

// ==========================================
// TESTE DE SOMA
// ==========================================
void teste_soma(int dim, double esp) {
    auto baseA = gerar_matriz_esparsa(dim, esp);
    auto baseB = gerar_matriz_esparsa(dim, esp); 

    Cronometro cron;
    long long t_densa = -1, t_e1 = 0, t_e2 = 0;
    long long m_densa = 0, m_e1 = 0, m_e2 = 0;

    if (dim <= 5000) { // Reduzido um pouco para garantir que não demore horas no loop grande
        start_tracking();
        {
            MatrizDensa A(dim, dim), B(dim, dim);
            for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
            for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking(); 
            cron.comecar();
            MatrizDensa C = A.somar(B);
            t_densa = cron.fim_ns();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    {
        MatrizEsparsaHashDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaHashDup C = A.somar(B);
        t_e1 = cron.fim_ns();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    {
        MatrizEsparsaTreeDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaTreeDup C = A.somar(B);
        t_e2 = cron.fim_ns();
        m_e2 = get_tracked_bytes();
        stop_tracking();
    }

    imprimir_csv("SOMA", "Densa", dim, esp, t_densa, m_densa);
    imprimir_csv("SOMA", "Est1(Hash)", dim, esp, t_e1, m_e1);
    imprimir_csv("SOMA", "Est2(Tree)", dim, esp, t_e2, m_e2);
}

// ==========================================
// TESTE DE MULTIPLICACAO
// ==========================================
void teste_multiplicacao(int dim, double esp) {
    bool rodar_densa = (dim <= 500); // Densa explode rápido aqui (N^3)

    auto baseA = gerar_matriz_esparsa(dim, esp);
    auto baseB = gerar_matriz_esparsa(dim, esp);

    Cronometro cron;
    long long t_densa = -1, t_e1 = 0, t_e2 = 0;
    long long m_densa = 0, m_e1 = 0, m_e2 = 0;

    if (rodar_densa) {
        MatrizDensa A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);
        
        start_tracking();
        cron.comecar();
        MatrizDensa C = A.multiplicar(B);
        t_densa = cron.fim_ns();
        m_densa = get_tracked_bytes();
        stop_tracking();
    }

    {
        MatrizEsparsaHashDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaHashDup C = A.multiplicar(B);
        t_e1 = cron.fim_ns();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    {
        MatrizEsparsaTreeDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaTreeDup C = A.multiplicar(B);
        t_e2 = cron.fim_ns();
        m_e2 = get_tracked_bytes();
        stop_tracking();
    }

    imprimir_csv("MULT", "Densa", dim, esp, t_densa, m_densa);
    imprimir_csv("MULT", "Est1(Hash)", dim, esp, t_e1, m_e1);
    imprimir_csv("MULT", "Est2(Tree)", dim, esp, t_e2, m_e2);
}

// ==========================================
// TESTE DE ESCALAR
// ==========================================
void teste_escalar(int dim, double esp) {
    double escalar = 3.14;
    auto base = gerar_matriz_esparsa(dim, esp);

    Cronometro cron;
    long long t_densa = -1, t_e1 = 0, t_e2 = 0;
    long long m_densa = 0, m_e1 = 0, m_e2 = 0;

    if (dim <= 10000) { 
        start_tracking();
        {
            MatrizDensa A(dim, dim);
            for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking(); 
            cron.comecar();
            A.multiplicarEscalarInPlace(escalar);
            t_densa = cron.fim_ns();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    {
        MatrizEsparsaHashDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.multiplicarEscalar(escalar); 
        t_e1 = cron.fim_ns();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    {
        MatrizEsparsaTreeDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.multiplicarEscalar(escalar);
        t_e2 = cron.fim_ns();
        m_e2 = get_tracked_bytes();
        stop_tracking();
    }

    imprimir_csv("ESCALAR", "Densa", dim, esp, t_densa, m_densa);
    imprimir_csv("ESCALAR", "Est1(Hash)", dim, esp, t_e1, m_e1);
    imprimir_csv("ESCALAR", "Est2(Tree)", dim, esp, t_e2, m_e2);
}

void teste_todas_operacoes() {
    srand(time(NULL));

    // Loop para cada operação
    // Nota: Mantive os loops separados para garantir que o CSV fique agrupado por operação
    
    // 1. Transposta
    for (int i = 2; i <= 8; i++) {
        double esparsidades[4];
        long long dimensao = pow(10, i);
        if (i < 4) {
            esparsidades[0] = 0.01; esparsidades[1] = 0.05; esparsidades[2] = 0.10; esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }
        for (double e : esparsidades) { if (e == 0.0) continue; teste_transposta(dimensao, e); }
    }

    // 2. Soma
    for (int i = 2; i <= 8; i++) {
        double esparsidades[4];
        long long dimensao = pow(10, i);
        if (i < 4) {
            esparsidades[0] = 0.01; esparsidades[1] = 0.05; esparsidades[2] = 0.10; esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }
        for (double e : esparsidades) { if (e == 0.0) continue; teste_soma(dimensao, e); }
    }

    // 3. Multiplicação
    for (int i = 2; i <= 8; i++) {
        double esparsidades[4];
        long long dimensao = pow(10, i);
        if (i < 4) {
            esparsidades[0] = 0.01; esparsidades[1] = 0.05; esparsidades[2] = 0.10; esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }
        for (double e : esparsidades) { if (e == 0.0) continue; teste_multiplicacao(dimensao, e); }
    }

    // 4. Escalar
    for (int i = 2; i <= 8; i++) {
        double esparsidades[4];
        long long dimensao = pow(10, i);
        if (i < 4) {
            esparsidades[0] = 0.01; esparsidades[1] = 0.05; esparsidades[2] = 0.10; esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }
        for (double e : esparsidades) { if (e == 0.0) continue; teste_escalar(dimensao, e); }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    // Imprime o Cabeçalho do CSV
    cout << "Operacao,Estrutura,N,Esparsidade,Tempo_ns,Memoria_Bytes" << endl;

    teste_todas_operacoes();

    return 0;
}