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

using namespace std;

const int N = 20000;   // dimensão fixa
const int TRIALS = 3;   // repetir para mediana

void imprimir_csv(
    const string &op,          // SOMA ou MULT
    const string &estrutura,   // Hash ou Tree
    long long k,               // número de elementos
    long long tempo,    // tempo em ns
    long long mem       // memória usada
) {
    cout << op << "," << estrutura << "," << k << "," << tempo << "," << mem << '\n';
}

// -----------------------------
// Amostragens: Floyd's algorithm
// -----------------------------
static vector<uint64_t> sample_k_unique_indices(uint64_t total, uint64_t k, std::mt19937_64 &rng) {
    // Floyd's algorithm (efficient, O(k))
    unordered_set<uint64_t> S;
    S.reserve((size_t) (k * 1.3 + 10));
    for (uint64_t i = total - k; i < total; ++i) {
        uniform_int_distribution<uint64_t> dist(0, i);
        uint64_t r = dist(rng);
        if (S.find(r) == S.end()) S.insert(r);
        else S.insert(i);
    }
    vector<uint64_t> out;
    out.reserve(S.size());
    for (auto v : S) out.push_back(v);
    return out;
}

vector<uint64_t> gerar_lista_k_super_denso() {
    vector<uint64_t> ks;
    ks.reserve(5000); // segurança

    // 1) ultra denso (1 a 200, passo 1)
    for (uint64_t x = 1; x <= 200; x++)
        ks.push_back(x);

    // 2) super denso (200 a 2000, passo 1)
    for (uint64_t x = 201; x <= 2000; x++)
        ks.push_back(x);

    // 3) denso moderado (2000 a 10000, passo 10)
    for (uint64_t x = 2010; x <= 10000; x += 10)
        ks.push_back(x);

    // 4) médio (10000 a 50000, passo 100)
    for (uint64_t x = 10100; x <= 50000; x += 100)
        ks.push_back(x);

    // 5) pesado (50k até 2M)
    ks.push_back(50000);
    ks.push_back(100000);
    ks.push_back(300000);
    ks.push_back(500000);
    ks.push_back(1000000);
    ks.push_back(2000000);

    return ks;
}

struct EntryLocal { int i; int j; int valor; };

static vector<EntryLocal> generate_exact_k_entries(int Nlocal, uint64_t k, std::mt19937_64 &rng) {
    uint64_t total = (uint64_t)Nlocal * (uint64_t)Nlocal;
    if (k > total) k = total;
    vector<EntryLocal> entries;
    entries.reserve((size_t)k);

    if (k == 0) return entries;

    // If k is extremely large (close to total) avoid Floyd; but in our experiments k << total
    auto idxs = sample_k_unique_indices(total, k, rng);
    for (uint64_t idx : idxs) {
        int ii = (int)(idx / Nlocal);
        int jj = (int)(idx % Nlocal);
        entries.push_back(EntryLocal{ii, jj, (int)(rng() % 100 + 1)});
    }
    return entries;
}

// ---------------------------------------------------
// Teste de SOMA (mediana de TRIALS)
// ---------------------------------------------------
void teste_soma_k(uint64_t k, std::mt19937_64 &rng) {
    // prepare entries once
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
            long long tn = cron.fim_ns();
            long long mem = get_tracked_bytes();
            stop_tracking();

            times_hash.push_back(tn);
            mems_hash.push_back(mem);
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
            long long tn = cron.fim_ns();
            long long mem = get_tracked_bytes();
            stop_tracking();

            times_tree.push_back(tn);
            mems_tree.push_back(mem);
        }
    }

    sort(times_hash.begin(), times_hash.end());
    sort(mems_hash.begin(), mems_hash.end());
    sort(times_tree.begin(), times_tree.end());
    sort(mems_tree.begin(), mems_tree.end());

    long long med_time_hash = times_hash[TRIALS/2];
    long long med_mem_hash  = mems_hash[TRIALS/2];
    long long med_time_tree = times_tree[TRIALS/2];
    long long med_mem_tree  = mems_tree[TRIALS/2];

    imprimir_csv("SOMA", "Hash", (int)k, med_time_hash, med_mem_hash);
    imprimir_csv("SOMA", "Tree", (int)k, med_time_tree, med_mem_tree);
}

// ---------------------------------------------------
// Teste de MULTIPLICAÇÃO (mediana de TRIALS)
// ---------------------------------------------------
void teste_mult_k(uint64_t k, std::mt19937_64 &rng) {
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
            long long tn = cron.fim_ns();
            long long mem = get_tracked_bytes();
            stop_tracking();

            times_hash.push_back(tn);
            mems_hash.push_back(mem);
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
            long long tn = cron.fim_ns();
            long long mem = get_tracked_bytes();
            stop_tracking();

            times_tree.push_back(tn);
            mems_tree.push_back(mem);
        }
    }

    sort(times_hash.begin(), times_hash.end());
    sort(mems_hash.begin(), mems_hash.end());
    sort(times_tree.begin(), times_tree.end());
    sort(mems_tree.begin(), mems_tree.end());

    long long med_time_hash = times_hash[TRIALS/2];
    long long med_mem_hash  = mems_hash[TRIALS/2];
    long long med_time_tree = times_tree[TRIALS/2];
    long long med_mem_tree  = mems_tree[TRIALS/2];

    imprimir_csv("MULT", "Hash", (int)k, med_time_hash, med_mem_hash);
    imprimir_csv("MULT", "Tree", (int)k, med_time_tree, med_mem_tree);
}

// ---------------------------------------------------
// New ambitious list of k values (dense at start, then large)
// ---------------------------------------------------
vector<uint64_t> gerar_lista_k_ambiciosa() {
    vector<uint64_t> ks;
    // very fine granularity initially
    for (uint64_t x = 10; x <= 200; x += 10) ks.push_back(x);
    for (uint64_t x = 300; x <= 1000; x += 100) ks.push_back(x);
    // medium
    for (uint64_t x = 2000; x <= 20000; x += 2000) ks.push_back(x);
    // large
    ks.push_back(50000);
    ks.push_back(100000);
    ks.push_back(300000);
    ks.push_back(500000);
    ks.push_back(1000000);
    ks.push_back(2000000);
    ks.push_back(5000000);
    ks.push_back(10000000);
    return ks;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)time(nullptr));

    std::random_device rd; std::mt19937_64 rng(rd());

    cout << "Operacao,Estrutura,k,Tempo_ns,Memoria_Bytes" << '\n';

    //auto ks = gerar_lista_k_ambiciosa();
    auto ks = gerar_lista_k_super_denso();

    for (auto k : ks) {
        // safety: skip if k > N*N
        uint64_t total = (uint64_t)N * (uint64_t)N;
        if (k > total) break;

        cerr << "Running k=" << k << "\n"; cerr.flush();

        // run soma then mult
        teste_soma_k(k, rng);
        teste_mult_k(k, rng);

        // flush to file
        cout.flush();
    }

    return 0;
}
