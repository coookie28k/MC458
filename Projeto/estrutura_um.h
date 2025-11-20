#include <stdio.h>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <unordered_map>
#include <algorithm> 
using namespace std;

/*
    -------------
    [ESTRUTURA 1]
    -------------
*/

struct Node1 {
    int i, j;
    double valor;

    // ponteiros para representação IJ (linha/coluna)
    Node1* prevRowIJ;
    Node1* nextRowIJ;
    Node1* prevColIJ;
    Node1* nextColIJ;

    // ponteiros para representação JI (linha/coluna da transposta)
    Node1* prevRowJI;
    Node1* nextRowJI;
    Node1* prevColJI;
    Node1* nextColJI;

    Node1(int _i, int _j, double _valor)
        : i(_i), j(_j), valor(_valor),
          prevRowIJ(nullptr), nextRowIJ(nullptr), prevColIJ(nullptr), nextColIJ(nullptr),
          prevRowJI(nullptr), nextRowJI(nullptr), prevColJI(nullptr), nextColJI(nullptr) {}
};

class MatrizEsparsaHashDup {
private:
    int linhas_, colunas_;

    // duas tabelas hash: IJ (chave (i,j)) e JI (chave (j,i))
    unordered_map<uint64_t, Node1*> tabelaIJ;
    unordered_map<uint64_t, Node1*> tabelaJI;

    // listas de cabeças:
    // para representação IJ:
    vector<Node1*> headsRowIJ;   // size = linhas_
    vector<Node1*> headsColIJ;   // size = colunas_
    // para representação JI:
    vector<Node1*> headsRowJI;   // size = colunas_  (rows of JI correspond to cols of IJ)
    vector<Node1*> headsColJI;   // size = linhas_

    // ponteiros "ativos" que definem a visão lógica atual (trocam em transpor)
    unordered_map<uint64_t, Node1*>* tabelaAtiva;
    vector<Node1*>* headsRowAtiva;
    vector<Node1*>* headsColAtiva;

    // ponteiros para as estruturas físicas (para facilitar inserção/remoção)
    unordered_map<uint64_t, Node1*>* tabelaFisicaIJ;
    unordered_map<uint64_t, Node1*>* tabelaFisicaJI;

    // helpers para manipular chaves
    static inline uint64_t keyIJ(int i, int j) {
        return (((uint64_t)(uint32_t)i) << 32) | (uint32_t)j;
    }
    static inline uint64_t keyJI(int j, int i) {
        return (((uint64_t)(uint32_t)j) << 32) | (uint32_t)i;
    }

    // helpers para identificar qual tabela ativa (apenas comparações de ponteiro)
    bool activeIsIJ() const { return tabelaAtiva == tabelaFisicaIJ; }

public:
    MatrizEsparsaHashDup(int linhas, int colunas)
        : linhas_(linhas), colunas_(colunas),
          headsRowIJ(max(1, linhas), nullptr),
          headsColIJ(max(1, colunas), nullptr),
          headsRowJI(max(1, colunas), nullptr),
          headsColJI(max(1, linhas), nullptr)
    {
        tabelaFisicaIJ = &tabelaIJ;
        tabelaFisicaJI = &tabelaJI;

        // inicializar visão ativa para IJ (matriz normal)
        tabelaAtiva = tabelaFisicaIJ;
        headsRowAtiva = &headsRowIJ;
        headsColAtiva = &headsColIJ;
    }

    ~MatrizEsparsaHashDup() {
        // deletar todos os Node1 alocados (cada Node está em tabelaIJ exatamente uma vez)
        for (auto &p : tabelaIJ) {
            delete p.second;
        }
        tabelaIJ.clear();
        tabelaJI.clear();
        // vectors serão liberados automaticamente
    }

    int getLinhas() const { return linhas_; }
    int getColunas() const { return colunas_; }

    // percorre a lista de "row ativa" retornando o próximo, conforme view ativa
    static Node1* nextRowActive(Node1* n, bool viewIsIJ) {
        return viewIsIJ ? n->nextRowIJ : n->nextRowJI;
    }
    static Node1* nextColActive(Node1* n, bool viewIsIJ) {
        return viewIsIJ ? n->nextColIJ : n->nextColJI;
    }

    // atualiza as cabeças ativas (interno) - chamado em transpor
    void setActiveToIJ() {
        tabelaAtiva = tabelaFisicaIJ;
        headsRowAtiva = &headsRowIJ;
        headsColAtiva = &headsColIJ;
    }
    void setActiveToJI() {
        tabelaAtiva = tabelaFisicaJI;
        headsRowAtiva = &headsRowJI;
        headsColAtiva = &headsColJI;
    }

    //INSERIR OU ATUALIZAR ELEMENTO
    void set(int i, int j, double valor) {
        if (i < 0 || j < 0) return;

        uint64_t kIJ = keyIJ(i,j);
        uint64_t kJI = keyJI(j,i);

        auto itIJ = tabelaIJ.find(kIJ);
        if (itIJ != tabelaIJ.end()) {
            // já existe Node físico (apontado por tabelaIJ)
            Node1* node = itIJ->second;
            if (valor == 0.0) {
                // REMOÇÃO O(1): desvincular das 4 listas e remover das 2 tabelas
                // remover de row IJ
                if (node->prevRowIJ) node->prevRowIJ->nextRowIJ = node->nextRowIJ;
                else headsRowIJ[node->i] = node->nextRowIJ;
                if (node->nextRowIJ) node->nextRowIJ->prevRowIJ = node->prevRowIJ;

                // remover de col IJ
                if (node->prevColIJ) node->prevColIJ->nextColIJ = node->nextColIJ;
                else headsColIJ[node->j] = node->nextColIJ;
                if (node->nextColIJ) node->nextColIJ->prevColIJ = node->prevColIJ;

                // remover de row JI (row = j)
                if (node->prevRowJI) node->prevRowJI->nextRowJI = node->nextRowJI;
                else headsRowJI[node->j] = node->nextRowJI;
                if (node->nextRowJI) node->nextRowJI->prevRowJI = node->prevRowJI;

                // remover de col JI (col = i)
                if (node->prevColJI) node->prevColJI->nextColJI = node->nextColJI;
                else headsColJI[node->i] = node->nextColJI;
                if (node->nextColJI) node->nextColJI->prevColJI = node->prevColJI;

                // apagar das tabelas
                tabelaIJ.erase(kIJ);
                tabelaJI.erase(kJI);
                delete node;
            } else {
                // atualização simples do valor
                node->valor = valor;
            }
            return;
        }

        // não existe e valor == 0 -> nada a fazer
        if (valor == 0.0) return;

        // criar novo Node físico e inserir nas 4 listas e 2 tabelas
        Node1* novo = new Node1(i, j, valor);

        // inserir em row IJ (no início)
        novo->nextRowIJ = headsRowIJ[i];
        if (headsRowIJ[i]) headsRowIJ[i]->prevRowIJ = novo;
        novo->prevRowIJ = nullptr;
        headsRowIJ[i] = novo;

        // inserir em col IJ (no início)
        novo->nextColIJ = headsColIJ[j];
        if (headsColIJ[j]) headsColIJ[j]->prevColIJ = novo;
        novo->prevColIJ = nullptr;
        headsColIJ[j] = novo;

        // inserir em row JI (no início) -> row index = j
        novo->nextRowJI = headsRowJI[j];
        if (headsRowJI[j]) headsRowJI[j]->prevRowJI = novo;
        novo->prevRowJI = nullptr;
        headsRowJI[j] = novo;

        // inserir em col JI (no início) -> col index = i
        novo->nextColJI = headsColJI[i];
        if (headsColJI[i]) headsColJI[i]->prevColJI = novo;
        novo->prevColJI = nullptr;
        headsColJI[i] = novo;

        // registrar nas duas tabelas (mesmo Node*)
        tabelaIJ[kIJ] = novo;
        tabelaJI[kJI] = novo;
    }

    //ACESSAR ELEMENTO
    double getElemento(int i, int j) const {
        if (i < 0 || j < 0) return 0.0;
        // lookup sempre via tabelaIJ (sem usar flags) — porém usamos tabelaAtiva para suportar transpor O(1)
        if (tabelaAtiva == &tabelaIJ) {
            uint64_t k = keyIJ(i,j);
            auto it = tabelaIJ.find(k);
            return (it == tabelaIJ.end() ? 0.0 : it->second->valor);
        } else {
            // tabela ativa é JI: a visão lógica deseja elemento (i,j) mas tabelaJI é indexada por (j,i)
            uint64_t k = keyJI(j,i);
            auto it = tabelaJI.find(k);
            return (it == tabelaJI.end() ? 0.0 : it->second->valor);
        }
    }

    //RETORNAR TRANSPOSTA
    void transpor() {
        // trocar visão ativa em O(1): tabelas ativas e vetores de heads
        if (tabelaAtiva == &tabelaIJ) {
            tabelaAtiva = &tabelaJI;
            headsRowAtiva = &headsRowJI;
            headsColAtiva = &headsColJI;
        } else {
            tabelaAtiva = &tabelaIJ;
            headsRowAtiva = &headsRowIJ;
            headsColAtiva = &headsColIJ;
        }
        swap(linhas_, colunas_);
    }

    //SOMA DE MATRIZES
    MatrizEsparsaHashDup somar(const MatrizEsparsaHashDup& B) const {
        // respeitamos a visão lógica atual (headsRowAtiva) de cada matriz
        MatrizEsparsaHashDup C(linhas_, colunas_);

        // Iterar por linhas lógicas (usar headsRowAtiva)
        bool viewIsIJ_A = (this->tabelaAtiva == &this->tabelaIJ);
        bool viewIsIJ_B = (B.tabelaAtiva == &B.tabelaIJ);

        // Percorrer A:
        vector<Node1*> const &headsA = *(this->headsRowAtiva);
        for (int i = 0; i < (int)headsA.size(); ++i) {
            for (Node1* n = headsA[i]; n != nullptr; n = (viewIsIJ_A ? n->nextRowIJ : n->nextRowJI)) {
                int ii = viewIsIJ_A ? n->i : n->j;
                int jj = viewIsIJ_A ? n->j : n->i;
                C.set(ii, jj, n->valor);
            }
        }

        // Percorrer B:
        vector<Node1*> const &headsB = *(B.headsRowAtiva);
        for (int i = 0; i < (int)headsB.size(); ++i) {
            for (Node1* n = headsB[i]; n != nullptr; n = (viewIsIJ_B ? n->nextRowIJ : n->nextRowJI)) {
                int ii = viewIsIJ_B ? n->i : n->j;
                int jj = viewIsIJ_B ? n->j : n->i;
                double atual = C.getElemento(ii, jj);
                C.set(ii, jj, atual + n->valor);
            }
        }

        return C;
    }

    //MULTIPLICACAO POR ESCALAR
    MatrizEsparsaHashDup multiplicarEscalar(double escalar) const {
        MatrizEsparsaHashDup R(linhas_, colunas_);

        vector<Node1*> const &headsA = *(this->headsRowAtiva);
        bool viewIsIJ_A = (this->tabelaAtiva == &this->tabelaIJ);
        for (int i = 0; i < (int)headsA.size(); ++i) {
            for (Node1* n = headsA[i]; n != nullptr; n = (viewIsIJ_A ? n->nextRowIJ : n->nextRowJI)) {
                int ii = viewIsIJ_A ? n->i : n->j;
                int jj = viewIsIJ_A ? n->j : n->i;
                R.set(ii, jj, n->valor * escalar);
            }
        }
        return R;
    }

    //MULTIPLICACAO DE MATRIZES
    MatrizEsparsaHashDup multiplicar(const MatrizEsparsaHashDup& B) const {
        MatrizEsparsaHashDup C(linhas_, B.colunas_);

        // vistas lógicas
        vector<Node1*> const &headsA = *(this->headsRowAtiva);
        bool viewIsIJ_A = (this->tabelaAtiva == &this->tabelaIJ);
        vector<Node1*> const &headsB = *(B.headsRowAtiva);
        bool viewIsIJ_B = (B.tabelaAtiva == &B.tabelaIJ);

        // para cada A[i,k]
        for (int i = 0; i < (int)headsA.size(); ++i) {
            for (Node1* na = headsA[i]; na != nullptr; na = (viewIsIJ_A ? na->nextRowIJ : na->nextRowJI)) {
                int ai = viewIsIJ_A ? na->i : na->j;
                int ak = viewIsIJ_A ? na->j : na->i;
                // obter linha ak de B (via headsB)
                if (ak < 0 || ak >= (int)headsB.size()) continue;
                for (Node1* nb = headsB[ak]; nb != nullptr; nb = (viewIsIJ_B ? nb->nextRowIJ : nb->nextRowJI)) {
                    int bj = viewIsIJ_B ? nb->j : nb->i;
                    double atual = C.getElemento(ai, bj);
                    C.set(ai, bj, atual + na->valor * nb->valor);
                }
            }
        }

        return C;
    }

};
