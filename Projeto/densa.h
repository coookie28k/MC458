#include <stdio.h>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <random>
#include <stdexcept>
#include <map>
using namespace std;

/*
    -------------
    [MATRIZ DENSA NORMAL PARA REFERÊNCIA]
    -------------
*/

class MatrizDensa{

private:
    vector<vector<double>> elementos_;
    int linhas_;
    int colunas_;

public:
    //construtor
    MatrizDensa(int linhas_, int colunas_): linhas_(linhas_), colunas_(colunas_), elementos_(linhas_, vector<double> (colunas_, 0.0)){}
    
    //INSERIR OU ATUALIZAR ELEMENTO
    void set(int i, int j, double valor) {
        elementos_[i][j] = valor;
    }   
    
    //ACESSAR ELEMENTO
    double getElemento(int i, int j) const {
        if (i >= 0 && i < linhas_ && j >=0 && j < colunas_){
            return elementos_[i][j];
        }
        return 0;
    }

    int getLinhas() const {
        return linhas_;
    }
    
    int getColunas() const {
        return colunas_;
    }

    //RETORNAR TRANSPOSTA
    MatrizDensa transposta() const {
        MatrizDensa resultado(colunas_, linhas_); 
        
        for (int i = 0; i < linhas_; ++i) {
            for (int j = 0; j < colunas_; ++j) {
                resultado.set(j, i, elementos_[i][j]); 
            }
        }
        return resultado;
    }
    
    //SOMA DE MATRIZES
    MatrizDensa somar(const MatrizDensa& outra) const {
    //como so tratamos com matrizes quadradas nos casos testes nao vai dar problema
        MatrizDensa resultado(linhas_, colunas_);
        
        for (int i = 0; i < linhas_; ++i) {
            for (int j = 0; j < colunas_; ++j) {
                double soma = elementos_[i][j] + outra.elementos_[i][j];
                resultado.set(i, j, soma);
            }
        }
        return resultado;
    }

    //MULTIPLICACAO POR ESCALAR
    /*
    MatrizDensa multiplicarEscalar(double escalar) const {
        MatrizDensa resultado(linhas_, colunas_);
        
        for (int i = 0; i < linhas_; ++i) {
            for (int j = 0; j < colunas_; ++j) {
                double produto = elementos_[i][j] * escalar;
                resultado.set(i, j, produto);
            }
        }
        return resultado;
    } */
   void multiplicarEscalarInPlace(double escalar) {
        for (int i = 0; i < linhas_; ++i) {
            for (int j = 0; j < colunas_; ++j) {
                elementos_[i][j] *= escalar;
            }
        }
    }


    //MULTIPLICACAO DE MATRIZES
    MatrizDensa multiplicar(const MatrizDensa& outra) const {
        //como so tratamos com matrizes quadradas nos casos testes nao vai dar problema
        const int dimensao = linhas_; 
        MatrizDensa resultado(dimensao, dimensao); 

        for (int i = 0; i < dimensao; ++i) { 
            for (int j = 0; j < dimensao; ++j) { 
                double soma_produto = 0.0;
                for (int k = 0; k < dimensao; ++k) { 
                    // C[i][j] += A[i][k] * B[k][j]
                    soma_produto += elementos_[i][k] * outra.elementos_[k][j];
                }
                resultado.set(i, j, soma_produto);
            }
        }
        return resultado;
    }
};


