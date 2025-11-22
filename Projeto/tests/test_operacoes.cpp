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

// Limite máximo para executar Matriz Densa (evita estouro de RAM/Tempo)
const int LIMIT_DENSA = 10000;

void imprimir_csv(string op, string estrutura, int n, double esp, long long tempo, long long mem) {
    cout << op << "," 
         << estrutura << "," 
         << n << "," 
         << esp << "," 
         << tempo << "," 
         << mem << endl;
}

// ==========================================
// TESTE DE INSERCAO E CONSULTA
// ==========================================
void teste_insercao_consulta(int dim, double esp) {
    auto base = gerar_matriz_esparsa(dim, esp);
    
    vector<int> is; is.reserve(base.size());
    vector<int> js; js.reserve(base.size());
    vector<double> vals; vals.reserve(base.size());

    for(auto &p : base) {
        is.push_back(p.second.i);
        js.push_back(p.second.j);
        vals.push_back(p.second.valor);
    }
    
    size_t num_ops = is.size();
    if (num_ops == 0) return; 

    Cronometro cron;
    volatile double dummy = 0;

    // --- Densa ---
    long long t_set_densa = -1, m_set_densa = 0;
    long long t_get_densa = -1;

    if (dim <= LIMIT_DENSA) { 
        // Para medir memória total (Estrutura + Dados), iniciamos tracking ANTES do construtor
        start_tracking();
        {
            MatrizDensa A(dim, dim); 
            
            // Medir Tempo SET (inclui apenas a operação de setar valores)
            cron.comecar();
            for(size_t k=0; k<num_ops; k++) {
                A.set(is[k], js[k], vals[k]);
            }
            t_set_densa = cron.finalizar();
            
            // Captura memória total alocada (Matriz + Dados)
            m_set_densa = get_tracked_bytes(); 
            
            // Medir Tempo GET
            cron.comecar();
            for(size_t k=0; k<num_ops; k++) {
                dummy = A.getElemento(is[k], js[k]);
            }
            t_get_densa = cron.finalizar();
        }
        stop_tracking();
    }

    // --- Estrutura 1 (Hash) ---
    long long t_set_e1 = 0, m_set_e1 = 0;
    long long t_get_e1 = 0;
    {
        start_tracking();
        {
            MatrizEsparsaHashDup A(dim, dim);
            
            cron.comecar();
            for(size_t k=0; k<num_ops; k++) {
                A.set(is[k], js[k], vals[k]);
            }
            t_set_e1 = cron.finalizar();
            m_set_e1 = get_tracked_bytes();

            cron.comecar();
            for(size_t k=0; k<num_ops; k++) {
                dummy = A.getElemento(is[k], js[k]);
            }
            t_get_e1 = cron.finalizar();
        }
        stop_tracking();
    }

    // --- Estrutura 2 (Tree) ---
    long long t_set_e2 = 0, m_set_e2 = 0;
    long long t_get_e2 = 0;
    {
        start_tracking();
        {
            MatrizEsparsaTreeDup A(dim, dim);
            
            cron.comecar();
            for(size_t k=0; k<num_ops; k++) {
                A.set(is[k], js[k], vals[k]);
            }
            t_set_e2 = cron.finalizar();
            m_set_e2 = get_tracked_bytes();

            cron.comecar();
            for(size_t k=0; k<num_ops; k++) {
                dummy = A.getElemento(is[k], js[k]);
            }
            t_get_e2 = cron.finalizar();
        }
        stop_tracking();
    }

    imprimir_csv("SET", "Densa", dim, esp, t_set_densa, m_set_densa);
    imprimir_csv("SET", "Est1(Hash)", dim, esp, t_set_e1, m_set_e1);
    imprimir_csv("SET", "Est2(Tree)", dim, esp, t_set_e2, m_set_e2);

    imprimir_csv("GET", "Densa", dim, esp, t_get_densa, 0);
    imprimir_csv("GET", "Est1(Hash)", dim, esp, t_get_e1, 0);
    imprimir_csv("GET", "Est2(Tree)", dim, esp, t_get_e2, 0);
}

// ==========================================
// TESTE DA TRANSPOSTA
// ==========================================
void teste_transposta(int dim, double esp) {
    auto base = gerar_matriz_esparsa(dim, esp);
    Cronometro cron;

    // --- Densa ---
    long long t_densa = -1, m_densa = 0;
    if (dim <= LIMIT_DENSA) { 
        start_tracking();
        {
            MatrizDensa A(dim, dim);
            for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking(); 
            cron.comecar();
            MatrizDensa T = A.transposta();
            t_densa = cron.finalizar();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    // --- Hash ---
    long long t_e1 = 0, m_e1 = 0;
    {
        MatrizEsparsaHashDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.transpor(); 
        t_e1 = cron.finalizar();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    // --- Tree ---
    long long t_e2 = 0, m_e2 = 0;
    {
        MatrizEsparsaTreeDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.transpor();
        t_e2 = cron.finalizar();
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

    long long t_densa = -1, m_densa = 0;
    if (dim <= LIMIT_DENSA) { 
        start_tracking();
        {
            MatrizDensa A(dim, dim), B(dim, dim);
            for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
            for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking(); 
            cron.comecar();
            MatrizDensa C = A.somar(B);
            t_densa = cron.finalizar();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    long long t_e1 = 0, m_e1 = 0;
    {
        MatrizEsparsaHashDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaHashDup C = A.somar(B);
        t_e1 = cron.finalizar();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    long long t_e2 = 0, m_e2 = 0;
    {
        MatrizEsparsaTreeDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaTreeDup C = A.somar(B);
        t_e2 = cron.finalizar();
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
    int limit_mult_densa = 1000; 

    auto baseA = gerar_matriz_esparsa(dim, esp);
    auto baseB = gerar_matriz_esparsa(dim, esp);
    Cronometro cron;

    long long t_densa = -1, m_densa = 0;
    if (dim <= limit_mult_densa) {
        start_tracking();
        {
            MatrizDensa A(dim, dim), B(dim, dim);
            for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
            for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking();
            cron.comecar();
            MatrizDensa C = A.multiplicar(B);
            t_densa = cron.finalizar();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    long long t_e1 = 0, m_e1 = 0;
    {
        MatrizEsparsaHashDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaHashDup C = A.multiplicar(B);
        t_e1 = cron.finalizar();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    long long t_e2 = 0, m_e2 = 0;
    {
        MatrizEsparsaTreeDup A(dim, dim), B(dim, dim);
        for(auto &p : baseA) A.set(p.second.i, p.second.j, p.second.valor);
        for(auto &p : baseB) B.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        MatrizEsparsaTreeDup C = A.multiplicar(B);
        t_e2 = cron.finalizar();
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

    long long t_densa = -1, m_densa = 0;
    if (dim <= LIMIT_DENSA) { 
        start_tracking();
        {
            MatrizDensa A(dim, dim);
            for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);
            
            start_tracking(); 
            cron.comecar();
            A.multiplicarEscalarInPlace(escalar);
            t_densa = cron.finalizar();
            m_densa = get_tracked_bytes();
        }
        stop_tracking();
    }

    long long t_e1 = 0, m_e1 = 0;
    {
        MatrizEsparsaHashDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.multiplicarEscalar(escalar); 
        t_e1 = cron.finalizar();
        m_e1 = get_tracked_bytes();
        stop_tracking();
    }

    long long t_e2 = 0, m_e2 = 0;
    {
        MatrizEsparsaTreeDup A(dim, dim);
        for(auto &p : base) A.set(p.second.i, p.second.j, p.second.valor);

        start_tracking();
        cron.comecar();
        A.multiplicarEscalar(escalar);
        t_e2 = cron.finalizar();
        m_e2 = get_tracked_bytes();
        stop_tracking();
    }

    imprimir_csv("ESCALAR", "Densa", dim, esp, t_densa, m_densa);
    imprimir_csv("ESCALAR", "Est1(Hash)", dim, esp, t_e1, m_e1);
    imprimir_csv("ESCALAR", "Est2(Tree)", dim, esp, t_e2, m_e2);
}

void teste_todas_operacoes() {
    srand(time(NULL));

    for (int i = 2; i <= 8; i++) { // 10^2 = 100 ... 10^8
        double esparsidades[4];
        long long dimensao = pow(10, i);
        
        // Ajuste de esparsidade conforme especificacao do projeto
        if (i < 4) {
            esparsidades[0] = 0.01; esparsidades[1] = 0.05; esparsidades[2] = 0.10; esparsidades[3] = 0.20;
        } else {
            esparsidades[0] = (1.0 / pow(10, i+2)) / 100.0;
            esparsidades[1] = (1.0 / pow(10, i+1)) / 100.0;
            esparsidades[2] = (1.0 / pow(10, i))   / 100.0;
            esparsidades[3] = 0.0;
        }

        for (double e : esparsidades) { 
            if (e <= 0.0) continue; 
            
            teste_transposta(dimensao, e);
            teste_soma(dimensao, e);
            teste_multiplicacao(dimensao, e);
            teste_escalar(dimensao, e);
            teste_insercao_consulta(dimensao, e);
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    cout << "Operacao,Estrutura,N,Esparsidade,Tempo_ns,Memoria_Bytes" << endl;

    teste_todas_operacoes();

    return 0;
}