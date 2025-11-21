#include "../estrutura_um.h"      // MatrizEsparsaHashDup
#include "../estrutura_dois.h"    // MatrizEsparsaTreeDup
#include "../gerador.h"
#include "util_medicao.h"

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#include <unordered_set>
#include <algorithm> 

using namespace std;

const int N = 20000;   // dimensão fixa
const int TRIALS = 3;   // repetir para mediana

// Função auxiliar de impressão
void imprimir_csv(
    const string &op,          
    const string &estrutura,   
    long long k,               
    long long tempo,           
    long long mem              
) {
    // Cast explícito para double para garantir divisão de ponto flutuante
    double total_elementos = (double)N * (double)N;
    double esparsidade = (double)k / total_elementos;

    cout << op << "," 
         << estrutura << "," 
         << k << "," 
         << esparsidade << "," 
         << tempo << "," 
         << mem << '\n';
}

// -----------------------------
// Amostragens: Algoritmo de Floyd (Adaptado para long long)
// -----------------------------
static vector<long long> sample_k_unique_indices(long long total, long long k, std::mt19937_64 &rng) {
    unordered_set<long long> S;
    // Reserva memória para evitar rehashing frequente
    S.reserve((size_t)(k * 1.3 + 10)); 

    for (long long i = total - k; i < total; ++i) {
        uniform_int_distribution<long long> dist(0, i);
        long long r = dist(rng);
        
        if (S.find(r) == S.end()) {
            S.insert(r);
        } else {
            S.insert(i);
        }
    }
    
    vector<long long> out;
    out.reserve(S.size());
    for (auto v : S) out.push_back(v);
    return out;
}

// Lista de K para teste (Foco em densidade crescente)
vector<long long> gerar_lista_k_super_denso() {
    vector<long long> ks;
    ks.reserve(5000);

    // 1) Ultra denso (k=1 a 200, passo 1)
    for (long long x = 1; x <= 200; x++) ks.push_back(x);
    
    // 2) Super denso (k=200 a 2000, passo 5)
    for (long long x = 201; x <= 1000; x += 10) ks.push_back(x);
    
    // 3) Moderado (k=2000 a 10000, passo 25)
    for (long long x = 2010; x <= 10000; x += 100) ks.push_back(x);
    
    // 4) Médio (k=10000 a 40000, passo 125)
    for (long long x = 10100; x <= 40000; x += 1000) ks.push_back(x);

    // 5) Grande (k=10000 a 200000, passo 125)
    for (long long x = 41000; x <= 200000; x += 10000) ks.push_back(x);

    return ks;
}

struct EntryLocal { int i; int j; int valor; };

// Gera as entradas da matriz garantindo exatamente K elementos únicos
static vector<EntryLocal> generate_exact_k_entries(int Nlocal, long long k, std::mt19937_64 &rng) {
    long long total = (long long)Nlocal * (long long)Nlocal;
    if (k > total) k = total;
    
    vector<EntryLocal> entries;
    entries.reserve((size_t)k);

    if (k == 0) return entries;

    // Gera índices únicos linearizados (0 a N*N - 1)
    auto idxs = sample_k_unique_indices(total, k, rng);
    
    for (long long idx : idxs) {
        int ii = (int)(idx / Nlocal);
        int jj = (int)(idx % Nlocal);
        // Valor aleatório entre 1 e 100
        entries.push_back(EntryLocal{ii, jj, (int)(rng() % 100 + 1)});
    }
    return entries;
}

// ============================================================================
// TESTES
// ============================================================================

// 1. SOMA
void teste_soma_k(long long k, std::mt19937_64 &rng) {
    auto entriesA = generate_exact_k_entries(N, k, rng);
    auto entriesB = generate_exact_k_entries(N, k, rng);

    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        // Hash
        {
            MatrizEsparsaHashDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            auto C = A.somar(B);
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        // Tree
        {
            MatrizEsparsaTreeDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            auto C = A.somar(B);
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("SOMA", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("SOMA", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

// 2. MULTIPLICAÇÃO
void teste_mult_k(long long k, std::mt19937_64 &rng) {
    auto entriesA = generate_exact_k_entries(N, k, rng);
    auto entriesB = generate_exact_k_entries(N, k, rng);

    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        // Hash
        {
            MatrizEsparsaHashDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            auto C = A.multiplicar(B);
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        // Tree
        {
            MatrizEsparsaTreeDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            auto C = A.multiplicar(B);
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("MULT", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("MULT", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

// 3. TRANSPOSTA
void teste_transposta_k(long long k, std::mt19937_64 &rng) {
    auto entries = generate_exact_k_entries(N, k, rng);

    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        // Hash
        {
            MatrizEsparsaHashDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            A.transpor(); 
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        // Tree
        {
            MatrizEsparsaTreeDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            A.transpor();
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("TRANS", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("TRANS", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

// 4. ESCALAR
void teste_escalar_k(long long k, std::mt19937_64 &rng) {
    auto entries = generate_exact_k_entries(N, k, rng);
    double escalar = 3.14;

    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        // Hash
        {
            MatrizEsparsaHashDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            A.multiplicarEscalar(escalar);
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        // Tree
        {
            MatrizEsparsaTreeDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);

            Cronometro cron;
            start_tracking();
            cron.comecar();
            A.multiplicarEscalar(escalar);
            long long tn = cron.finalizar();
            long long mem = get_tracked_bytes();
            stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("ESCALAR", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("ESCALAR", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

// 5. INSERÇÃO E CONSULTA
void teste_insercao_consulta_k(long long k, std::mt19937_64 &rng) {
    auto entries = generate_exact_k_entries(N, k, rng);
    volatile double dummy = 0;

    vector<long long> t_set_hash, t_get_hash, m_set_hash;
    vector<long long> t_set_tree, t_get_tree, m_set_tree;

    for (int t = 0; t < TRIALS; ++t) {
        // --- Hash ---
        {
            start_tracking();
            Cronometro cron;
            MatrizEsparsaHashDup A(N, N);
            
            // SET
            cron.comecar();
            for(const auto &e : entries) {
                A.set(e.i, e.j, e.valor);
            }
            long long time_set = cron.finalizar();
            long long mem_set = get_tracked_bytes();
            stop_tracking();

            t_set_hash.push_back(time_set);
            m_set_hash.push_back(mem_set);

            // GET
            cron.comecar();
            for(const auto &e : entries) {
                dummy = A.getElemento(e.i, e.j);
            }
            long long time_get = cron.finalizar();
            t_get_hash.push_back(time_get);
        }

        // --- Tree ---
        {
            start_tracking();
            Cronometro cron;
            MatrizEsparsaTreeDup A(N, N);
            
            // SET
            cron.comecar();
            for(const auto &e : entries) {
                A.set(e.i, e.j, e.valor);
            }
            long long time_set = cron.finalizar();
            long long mem_set = get_tracked_bytes();
            stop_tracking();

            t_set_tree.push_back(time_set);
            m_set_tree.push_back(mem_set);

            // GET
            cron.comecar();
            for(const auto &e : entries) {
                dummy = A.getElemento(e.i, e.j);
            }
            long long time_get = cron.finalizar();
            t_get_tree.push_back(time_get);
        }
    }

    // Ordenação
    sort(t_set_hash.begin(), t_set_hash.end());
    sort(m_set_hash.begin(), m_set_hash.end());
    sort(t_get_hash.begin(), t_get_hash.end());

    sort(t_set_tree.begin(), t_set_tree.end());
    sort(m_set_tree.begin(), m_set_tree.end());
    sort(t_get_tree.begin(), t_get_tree.end());

    // Imprimir
    imprimir_csv("SET", "Hash", k, t_set_hash[TRIALS/2], m_set_hash[TRIALS/2]);
    imprimir_csv("SET", "Tree", k, t_set_tree[TRIALS/2], m_set_tree[TRIALS/2]);
    
    // Memória de GET é 0
    imprimir_csv("GET", "Hash", k, t_get_hash[TRIALS/2], 0);
    imprimir_csv("GET", "Tree", k, t_get_tree[TRIALS/2], 0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    // Cast para unsigned int padrão do C++ para srand
    srand((unsigned int)time(nullptr));

    std::random_device rd; 
    std::mt19937_64 rng(rd());

    cout << "Operacao,Estrutura,k,Esparsidade,Tempo_ns,Memoria_Bytes" << '\n';

    auto ks = gerar_lista_k_super_denso();

    for (auto k : ks) {
        // Cálculo seguro com long long
        long long total = (long long)N * (long long)N;
        if (k > total) break;

        // Log de progresso na stderr para não sujar o CSV
        cerr << "Running k=" << k << "\n"; cerr.flush();

        teste_soma_k(k, rng);
        teste_mult_k(k, rng);
        teste_transposta_k(k, rng);
        teste_escalar_k(k, rng);
        teste_insercao_consulta_k(k, rng);

        cout.flush();
    }

    return 0;
}