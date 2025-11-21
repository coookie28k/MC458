#!/usr/bin/env python3
# fit_compare_models.py

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

INPUT_CSV = "resultado_reduzido_O3.csv"
OUT_DIR = "fit_comparison_outputs"
os.makedirs(OUT_DIR, exist_ok=True)

# Tenta carregar o CSV
if not os.path.exists(INPUT_CSV):
    print(f"Erro: Arquivo '{INPUT_CSV}' não encontrado.")
    exit(1)

df = pd.read_csv(INPUT_CSV)
# Limpeza básica
cols_needed = ["Operacao","Estrutura","k","Tempo_ns"]
if not all(c in df.columns for c in cols_needed):
    print(f"Erro: CSV deve conter colunas: {cols_needed}")
    exit(1)

df = df.dropna(subset=cols_needed)
df["k"] = pd.to_numeric(df["k"], errors="coerce")
df["Tempo_ns"] = pd.to_numeric(df["Tempo_ns"], errors="coerce")
df = df[df["k"] > 0] # Evita problemas com log(0)

# ------------------------------------------------------------------------------
# Funções de Ajuste (Regressão)
# ------------------------------------------------------------------------------

def linear_fit_ols(X, y, fit_intercept=True):
    """
    Ajuste Linear via Mínimos Quadrados (OLS).
    Retorna: (coeffs, preds, r2)
    """
    if fit_intercept:
        # Adiciona coluna de 1s para calcular o 'b' (intercepto)
        X_design = np.hstack([X, np.ones((X.shape[0], 1))])
    else:
        # Força b=0 (reta passa pela origem)
        X_design = X
        
    # lstsq resolve min ||Ax - y||^2
    coeffs, residuals, rank, s = np.linalg.lstsq(X_design, y, rcond=None)
    
    preds = X_design.dot(coeffs)
    
    # Cálculo R^2
    ss_res = np.sum((y - preds)**2)
    ss_tot = np.sum((y - np.mean(y))**2)
    
    # Se ss_tot for 0 (todos y iguais), r2 é 0 ou indefinido
    r2 = 1 - (ss_res / ss_tot) if ss_tot > 1e-9 else 0.0
    
    return coeffs, preds, r2

# ------------------------------------------------------------------------------
# Definição de Features (Teórica e Genérica)
# ------------------------------------------------------------------------------

def get_theoretical_feature(op, estrutura, k_array):
    """
    Retorna a matriz X (N x 1) baseada na teoria (Big-O).
    """
    k = np.array(k_array, dtype=float)
    # Evitar log(0)
    log_k = np.log(1.0 + k) 
    
    if op == "SOMA":
        if estrutura == "Hash":
            # Teoria: O(k)
            return k.reshape(-1, 1)
        else: # Tree
            # Teoria: O(k log k)
            return (k * log_k).reshape(-1, 1)
    elif op == "MULT":
        if estrutura == "Hash":
            # Teoria: O(k^2)
            return (k**2).reshape(-1, 1)
        else:
            # Teoria: O(k^2 log k)
            return (k**2 * log_k).reshape(-1, 1)
    else:
        # Default fallback linear
        return k.reshape(-1, 1)

def get_generic_features(k_array):
    """
    Gera uma matriz com múltiplos termos para tentar encontrar a melhor curva genérica.
    Termos: [log k, k, k log k, k^2, k^2 log k]
    O termo constante (1) será adicionado pelo linear_fit_ols se fit_intercept=True.
    """
    k = np.array(k_array, dtype=float)
    log_k = np.log(1.0 + k)
    
    # Empilha as colunas. 
    # Ordem dos coeficientes será a mesma desta lista.
    # Nota: Não incluímos Bias (1.0) aqui, deixamos o fit_intercept cuidar ou adicionamos manual.
    # Vamos deixar o fit_intercept=True cuidar do Bias, então retornamos apenas as variáveis.
    
    X_gen = np.vstack([
        log_k,              # Termo 0: log k
        k,                  # Termo 1: k
        k * log_k,          # Termo 2: k log k
        k**2,               # Termo 3: k^2
        k**2 * log_k        # Termo 4: k^2 log k
    ]).T
    
    feature_names = ["log(k)", "k", "k*log(k)", "k^2", "k^2*log(k)", "Intercept"]
    return X_gen, feature_names

# ------------------------------------------------------------------------------
# Loop Principal
# ------------------------------------------------------------------------------

ops = df["Operacao"].unique()

print(f"{'Operacao':<10} | {'Estrutura':<10} | {'Modelo':<20} | {'R2':<8} | {'Detalhes'}")
print("-" * 100)

for op in ops:
    for estrutura in ["Hash", "Tree"]:
        sub = df[(df["Operacao"] == op) & (df["Estrutura"] == estrutura)].copy()
        if sub.empty:
            continue
        
        # Ordenar para plotar linhas corretamente
        sub = sub.sort_values("k")
        k_vals = sub["k"].values
        y_vals = sub["Tempo_ns"].values
        
        # --- 1. Modelo Teórico com Intercepto (a*X + b) ---
        X_theo = get_theoretical_feature(op, estrutura, k_vals)
        coeffs_theo, preds_theo, r2_theo = linear_fit_ols(X_theo, y_vals, fit_intercept=True)
        
        # --- 2. Modelo Teórico SEM Intercepto (a*X) ---
        coeffs_noint, preds_noint, r2_noint = linear_fit_ols(X_theo, y_vals, fit_intercept=False)
        
        # --- 3. Modelo Genérico (Combinação linear de termos) ---
        X_gen, gen_names = get_generic_features(k_vals)
        # Usamos fit_intercept=True para permitir um termo constante na fórmula genérica
        coeffs_gen, preds_gen, r2_gen = linear_fit_ols(X_gen, y_vals, fit_intercept=True)
        
        # Formatar string da equação genérica
        # coeffs_gen tem tam 6 (5 features + 1 intercept no final)
        eqn_parts = []
        # As features
        for i in range(len(coeffs_gen)-1):
            c = coeffs_gen[i]
            # Apenas mostra termos com relevância mínima para não poluir (opcional)
            if abs(c) > 1e-9: 
                eqn_parts.append(f"{c:.2e}*{gen_names[i]}")
        # O intercepto
        b_gen = coeffs_gen[-1]
        eqn_parts.append(f"{b_gen:.2e} (Bias)")
        gen_eqn_str = " + ".join(eqn_parts)

        # --- Print Resultados ---
        print(f"{op:<10} | {estrutura:<10} | {'Teorico (aX+b)':<20} | {r2_theo:.4f}   | a={coeffs_theo[0]:.2e}, b={coeffs_theo[1]:.2e}")
        print(f"{'':<10} | {'':<10} | {'Teorico (aX)':<20}   | {r2_noint:.4f}   | a={coeffs_noint[0]:.2e}, b=0")
        print(f"{'':<10} | {'':<10} | {'Generico (Poly)':<20}  | {r2_gen:.4f}   | {gen_eqn_str[:80]}...") # Truncar para caber
        print("-" * 100)

        # --- Plotagem ---
        plt.figure(figsize=(10, 6))
        plt.title(f"{op} - {estrutura}: Comparação de Modelos")
        
        # Dados reais
        plt.scatter(k_vals, y_vals, color='black', s=15, alpha=0.6, label="Medido")
        
        # Linhas de predição
        plt.plot(k_vals, preds_theo, color='blue', linewidth=2, label=f"Teórico (aX+b) R2={r2_theo:.3f}")
        plt.plot(k_vals, preds_noint, color='green', linestyle='--', linewidth=2, label=f"Teórico (aX) R2={r2_noint:.3f}")
        plt.plot(k_vals, preds_gen, color='red', linestyle=':', linewidth=2, label=f"Genérico R2={r2_gen:.3f}")
        
        plt.xlabel("k (Elementos não nulos)")
        plt.ylabel("Tempo (ns)")
        plt.legend()
        plt.grid(True, alpha=0.3)
        
        filename = os.path.join(OUT_DIR, f"{op}_{estrutura}_models_comparison.png")
        plt.savefig(filename, dpi=150)
        plt.close()

print(f"\nGráficos salvos em '{OUT_DIR}'")