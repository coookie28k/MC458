#include <stdio.h>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <algorithm> 
#include <map>
using namespace std;

/*
    -------------
    [ESTRUTURA 2]
    -------------
*/

struct Node2 {
    int i, j;
    double valor;
    Node2(int _i, int _j, double v) : i(_i), j(_j), valor(v) {}
};

class MatrizEsparsaTreeDup {
private:
    int linhas_, colunas_;

    map<int, map<int, Node2*>> mapPorLinha;
    map<int, map<int, Node2*>> mapPorColuna;

    map<int, map<int, Node2*>>* linhaPtr;
    map<int, map<int, Node2*>>* colPtr;
    
    MatrizEsparsaTreeDup(const MatrizEsparsaTreeDup& original)
        : linhas_(original.linhas_), colunas_(original.colunas_)
    {
        linhaPtr = &mapPorLinha;
        colPtr = &mapPorColuna;
        
        for (auto const& [i, innerMap] : original.mapPorLinha) {
            for (auto const& [j, nodeOriginal] : innerMap) {
                Node2* novo = new Node2(i, j, nodeOriginal->valor);
                
                mapPorLinha[i][j] = novo;
                mapPorColuna[j][i] = novo;
            }
        }
        
        if (original.linhaPtr == &original.mapPorColuna) {
            swap(linhaPtr, colPtr);
        }
    }


public:
    //construtor 
    MatrizEsparsaTreeDup(int linhas, int colunas)
        : linhas_(linhas), colunas_(colunas)
    {
        linhaPtr = &mapPorLinha;
        colPtr = &mapPorColuna;
    }

    int getLinhas() const { return linhas_; }
    int getColunas() const { return colunas_; }

    // DESTRUTOR 
    ~MatrizEsparsaTreeDup() {
        for (auto itOuter = mapPorLinha.begin(); itOuter != mapPorLinha.end(); ++itOuter) {
            for (auto itInner = itOuter->second.begin(); itInner != itOuter->second.end(); ++itInner) {
                if (itInner->second) delete itInner->second;
            }
        }
    }

    //INSERIR OU ATUALIZAR ELEMENTO
    void set(int i, int j, double valor) {
        if (i < 0 || j < 0) return;

        auto itRowOuter = mapPorLinha.find(i);
        if (itRowOuter != mapPorLinha.end()) {
            auto itInner = itRowOuter->second.find(j);
            if (itInner != itRowOuter->second.end()) {
                Node2* n = itInner->second;
                if (valor == 0.0) {
                    mapPorLinha[i].erase(j);
                    if (mapPorLinha[i].empty()) mapPorLinha.erase(i);

                    mapPorColuna[j].erase(i);
                    if (mapPorColuna[j].empty()) mapPorColuna.erase(j);

                    delete n;
                } else {
                    n->valor = valor;
                }
                return;
            }
        }
        if (valor == 0.0) return;

        Node2* novo = new Node2(i, j, valor);
        mapPorLinha[i][j] = novo; 
        mapPorColuna[j][i] = novo; 
    }

    //ACESSAR ELEMENTO
    double getElemento(int i, int j) const {
        if (i < 0 || j < 0) return 0.0;
        auto itOuter = linhaPtr->find(i);
        if (itOuter == linhaPtr->end()) return 0.0;
        auto itInner = itOuter->second.find(j);
        if (itInner == itOuter->second.end()) return 0.0;
        return itInner->second->valor;
    }

    //RETORNAR TRANSPOSTA
    void transpor() {
        swap(linhaPtr, colPtr);
        swap(linhas_, colunas_);
    }

    //SOMA DE MATRIZES
    MatrizEsparsaTreeDup somar(const MatrizEsparsaTreeDup& B) const {
        MatrizEsparsaTreeDup C(linhas_, colunas_);

        for (auto itOuter = this->linhaPtr->begin(); itOuter != this->linhaPtr->end(); ++itOuter) {
            int i = itOuter->first;
            for (auto itInner = itOuter->second.begin(); itInner != itOuter->second.end(); ++itInner) {
                int j = itInner->first;
                double v = itInner->second->valor;
                C.set(i, j, v);
            }
        }

        for (auto itOuter = B.linhaPtr->begin(); itOuter != B.linhaPtr->end(); ++itOuter) {
            int i = itOuter->first;
            for (auto itInner = itOuter->second.begin(); itInner != itOuter->second.end(); ++itInner) {
                int j = itInner->first;
                double v = itInner->second->valor;
                double atual = C.getElemento(i, j);
                C.set(i, j, atual + v);
            }
        }
        return C;
    }

    //MULTIPLICACAO POR ESCALAR
    /*
    MatrizEsparsaTreeDup multiplicarEscalar(double escalar) const {
        MatrizEsparsaTreeDup R(*this);

        for (auto const& [i, innerMap] : R.mapPorLinha) {
            for (auto const& [j, node] : innerMap) {
                node->valor *= escalar;
            }
        }

        return R;
    }*/
    void multiplicarEscalar(double escalar) {
        for (auto& [i, inner] : mapPorLinha) {
            for (auto& [j, node] : inner) {
                node->valor *= escalar;
            }
        }
    }

    // MULTIPLICACAO DE MATRIZES
    MatrizEsparsaTreeDup multiplicar(const MatrizEsparsaTreeDup& B) const {
        MatrizEsparsaTreeDup C(this->linhas_, B.colunas_);

        for (auto itOuterA = this->linhaPtr->begin(); itOuterA != this->linhaPtr->end(); ++itOuterA) {
            int i = itOuterA->first;
            for (auto itInnerA = itOuterA->second.begin(); itInnerA != itOuterA->second.end(); ++itInnerA) {
                int k = itInnerA->first;
                double a_val = itInnerA->second->valor;

                auto rowBkIt = B.linhaPtr->find(k);
                if (rowBkIt == B.linhaPtr->end()) continue;

                for (auto itInnerB = rowBkIt->second.begin(); itInnerB != rowBkIt->second.end(); ++itInnerB) {
                    int j = itInnerB->first;
                    double b_val = itInnerB->second->valor;
                    double atual = C.getElemento(i, j);
                    C.set(i, j, atual + a_val * b_val);
                }
            }
        }
        return C;
    }
};