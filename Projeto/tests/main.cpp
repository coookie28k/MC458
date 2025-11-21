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
#include <omp.h> // Biblioteca OpenMP para paralelismo

using namespace std;

const int N = 20000;   // dimensão fixa (20k x 20k = 400 Milhões de células)
const int TRIALS = 3;  // repetir para mediana

// Função auxiliar de impressão Thread-Safe
void imprimir_csv(
    const string &op,          
    const string &estrutura,   
    long long k,               
    long long tempo,           
    long long mem              
) {
    double total_elementos = (double)N * (double)N;
    double esparsidade = (double)k / total_elementos;

    // Sessão crítica: Apenas uma thread escreve no terminal por vez
    #pragma omp critical
    {
        cout << op << "," 
             << estrutura << "," 
             << k << "," 
             << esparsidade << "," 
             << tempo << "," 
             << mem << endl; // endl força flush
    }
}

// -----------------------------
// Amostragens: Algoritmo de Floyd (Adaptado para long long)
// -----------------------------
static vector<long long> sample_k_unique_indices(long long total, long long k, std::mt19937_64 &rng) {
    unordered_set<long long> S;
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

// ---------------------------------------------------
// GERADOR DE PONTOS (Canhão de Dados)
// ---------------------------------------------------
vector<long long> gerar_lista_k_agressiva() {
    vector<long long> ks;
    ks.reserve(10000);
    unordered_set<long long> seen; // Para evitar duplicatas nos arredondamentos

    auto add_if_new = [&](long long val) {
        if (val > 0 && seen.find(val) == seen.end()) {
            ks.push_back(val);
            seen.insert(val);
        }
    };

    // 1. Nano-Escala (O Overhead inicial)
    // De 1 a 100, de 1 em 1
    for (long long k = 1; k <= 100; k++) add_if_new(k);

    // 2. Micro-Escala (Alta densidade para curvas iniciais)
    // De 100 a 2.000, passo 10
    for (long long k = 110; k <= 2000; k += 10) add_if_new(k);

    // 3. Escala Linear Fina
    // De 2.000 a 50.000, passo 500
    for (long long k = 2500; k <= 50000; k += 500) add_if_new(k);

    // 4. Escala Logarítmica/Exponencial (Cobre ordens de magnitude)
    // Cresce 10% a cada passo até 10 Milhões
    double curr = 50000;
    while (curr < 10000000) { // Vai até 10 Milhões de elementos
        curr *= 1.10; 
        add_if_new((long long)curr);
    }

    // 5. Pontos de "Stress Test" (Números redondos grandes)
    add_if_new(1000000); // 1M
    add_if_new(2000000); // 2M
    add_if_new(5000000); // 5M
    add_if_new(8000000); // 8M
    add_if_new(10000000); // 10M (Esparsidade 2.5% em 20k x 20k)

    // Ordena para o gráfico ficar bonito se plotar linha (opcional pois vamos processar paralelo)
    sort(ks.begin(), ks.end());
    
    return ks;
}

struct EntryLocal { int i; int j; int valor; };

static vector<EntryLocal> generate_exact_k_entries(int Nlocal, long long k, std::mt19937_64 &rng) {
    long long total = (long long)Nlocal * (long long)Nlocal;
    if (k > total) k = total;
    
    vector<EntryLocal> entries;
    entries.reserve((size_t)k);

    if (k == 0) return entries;

    auto idxs = sample_k_unique_indices(total, k, rng);
    
    for (long long idx : idxs) {
        int ii = (int)(idx / Nlocal);
        int jj = (int)(idx % Nlocal);
        entries.push_back(EntryLocal{ii, jj, (int)(rng() % 100 + 1)});
    }
    return entries;
}

// ============================================================================
// TESTES (Adaptados para receber RNG por referência)
// ============================================================================

void teste_soma_k(long long k, std::mt19937_64 &rng) {
    auto entriesA = generate_exact_k_entries(N, k, rng);
    auto entriesB = generate_exact_k_entries(N, k, rng);

    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        {
            MatrizEsparsaHashDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            auto C = A.somar(B);
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        {
            MatrizEsparsaTreeDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            auto C = A.somar(B);
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("SOMA", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("SOMA", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

void teste_mult_k(long long k, std::mt19937_64 &rng) {
    auto entriesA = generate_exact_k_entries(N, k, rng);
    auto entriesB = generate_exact_k_entries(N, k, rng);

    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        {
            MatrizEsparsaHashDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            auto C = A.multiplicar(B);
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        {
            MatrizEsparsaTreeDup A(N, N), B(N, N);
            for (auto &e : entriesA) A.set(e.i, e.j, e.valor);
            for (auto &e : entriesB) B.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            auto C = A.multiplicar(B);
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("MULT", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("MULT", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

void teste_transposta_k(long long k, std::mt19937_64 &rng) {
    auto entries = generate_exact_k_entries(N, k, rng);
    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        {
            MatrizEsparsaHashDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            A.transpor(); 
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        {
            MatrizEsparsaTreeDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            A.transpor();
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("TRANS", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("TRANS", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

void teste_escalar_k(long long k, std::mt19937_64 &rng) {
    auto entries = generate_exact_k_entries(N, k, rng);
    double escalar = 3.14;
    vector<long long> times_hash, mems_hash;
    vector<long long> times_tree, mems_tree;

    for (int t = 0; t < TRIALS; ++t) {
        {
            MatrizEsparsaHashDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            A.multiplicarEscalar(escalar);
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_hash.push_back(tn); mems_hash.push_back(mem);
        }
        {
            MatrizEsparsaTreeDup A(N, N);
            for (auto &e : entries) A.set(e.i, e.j, e.valor);
            Cronometro cron; start_tracking(); cron.comecar();
            A.multiplicarEscalar(escalar);
            long long tn = cron.finalizar(); long long mem = get_tracked_bytes(); stop_tracking();
            times_tree.push_back(tn); mems_tree.push_back(mem);
        }
    }
    sort(times_hash.begin(), times_hash.end());
    sort(times_tree.begin(), times_tree.end());
    imprimir_csv("ESCALAR", "Hash", k, times_hash[TRIALS/2], mems_hash[TRIALS/2]);
    imprimir_csv("ESCALAR", "Tree", k, times_tree[TRIALS/2], mems_tree[TRIALS/2]);
}

void teste_insercao_consulta_k(long long k, std::mt19937_64 &rng) {
    auto entries = generate_exact_k_entries(N, k, rng);
    volatile double dummy = 0;
    vector<long long> t_set_hash, t_get_hash, m_set_hash;
    vector<long long> t_set_tree, t_get_tree, m_set_tree;

    for (int t = 0; t < TRIALS; ++t) {
        {
            start_tracking(); Cronometro cron; MatrizEsparsaHashDup A(N, N);
            cron.comecar();
            for(const auto &e : entries) A.set(e.i, e.j, e.valor);
            long long time_set = cron.finalizar(); long long mem_set = get_tracked_bytes(); stop_tracking();
            t_set_hash.push_back(time_set); m_set_hash.push_back(mem_set);
            cron.comecar();
            for(const auto &e : entries) dummy = A.getElemento(e.i, e.j);
            long long time_get = cron.finalizar();
            t_get_hash.push_back(time_get);
        }
        {
            start_tracking(); Cronometro cron; MatrizEsparsaTreeDup A(N, N);
            cron.comecar();
            for(const auto &e : entries) A.set(e.i, e.j, e.valor);
            long long time_set = cron.finalizar(); long long mem_set = get_tracked_bytes(); stop_tracking();
            t_set_tree.push_back(time_set); m_set_tree.push_back(mem_set);
            cron.comecar();
            for(const auto &e : entries) dummy = A.getElemento(e.i, e.j);
            long long time_get = cron.finalizar();
            t_get_tree.push_back(time_get);
        }
    }
    sort(t_set_hash.begin(), t_set_hash.end()); sort(t_get_hash.begin(), t_get_hash.end());
    sort(t_set_tree.begin(), t_set_tree.end()); sort(t_get_tree.begin(), t_get_tree.end());

    imprimir_csv("SET", "Hash", k, t_set_hash[TRIALS/2], m_set_hash[TRIALS/2]);
    imprimir_csv("SET", "Tree", k, t_set_tree[TRIALS/2], m_set_tree[TRIALS/2]);
    imprimir_csv("GET", "Hash", k, t_get_hash[TRIALS/2], 0);
    imprimir_csv("GET", "Tree", k, t_get_tree[TRIALS/2], 0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Operacao,Estrutura,k,Esparsidade,Tempo_ns,Memoria_Bytes" << endl;

    auto ks = gerar_lista_k_agressiva();
    long long total_n2 = (long long)N * (long long)N;

    // Configuração do Paralelismo:
    // - schedule(dynamic, 1): Pega 1 tarefa (1 valor de k) por vez.
    //   Ideal pois ks pequenos são instantâneos e ks grandes demoram.
    // - num_threads(20): Usa todo o poder do seu i5-14600KF
    #pragma omp parallel for schedule(dynamic, 1) num_threads(20)
    for (size_t i = 0; i < ks.size(); ++i) {
        long long k = ks[i];
        
        if (k > total_n2) continue;

        // RNG Local por Thread para evitar Race Condition e gargalos
        // Seed única baseada no tempo + ID da thread + índice do loop
        std::hash<std::thread::id> hasher;
        unsigned long seed = time(nullptr) + hasher(std::this_thread::get_id()) + i;
        std::mt19937_64 thread_rng(seed);

        #pragma omp critical(console)
        {
            cerr << "[Thread " << omp_get_thread_num() << "] Processando k=" << k << endl;
        }

        teste_soma_k(k, thread_rng);
        teste_mult_k(k, thread_rng);
        teste_transposta_k(k, thread_rng);
        teste_escalar_k(k, thread_rng);
        teste_insercao_consulta_k(k, thread_rng);
    }

    return 0;
}