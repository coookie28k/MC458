#!/usr/bin/env python3
# fit_time_models.py
# Requer: pandas, numpy, matplotlib
# Ex: python3 fit_time_models.py

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from math import log

INPUT_CSV = "resultado_reduzido_O3.csv"
OUT_DIR = "fit_outputs"
os.makedirs(OUT_DIR, exist_ok=True)

df = pd.read_csv(INPUT_CSV)
df = df.dropna(subset=["Operacao","Estrutura","k","Tempo_ns"])
df["k"] = pd.to_numeric(df["k"], errors="coerce")
df["Tempo_ns"] = pd.to_numeric(df["Tempo_ns"], errors="coerce")

# Função utilitária de ajuste linear Y = a * X + b com LS (X pode ser matriz)
def linear_fit(X, y):
    # X: design matrix (n x m) without bias column
    # add column de 1 para intercepto
    Xb = np.hstack([X, np.ones((X.shape[0],1))])
    coeffs, *_ = np.linalg.lstsq(Xb, y, rcond=None)
    # coeffs is [a1, a2, ..., b]
    preds = Xb.dot(coeffs)
    # R^2:
    ss_res = np.sum((y - preds)**2)
    ss_tot = np.sum((y - np.mean(y))**2)
    r2 = 1 - ss_res/ss_tot if ss_tot > 0 else 0.0
    return coeffs, preds, r2

# Power-law fit: y = A * k^p  -> log y = log A + p log k
def power_law_fit(k, y):
    mask = (k > 0) & (y > 0)
    k2 = k[mask]
    y2 = y[mask]
    if len(k2) < 3:
        return None, None, None
    Lk = np.log(k2)
    Ly = np.log(y2)
    A = np.vstack([Lk, np.ones_like(Lk)]).T
    sol, *_ = np.linalg.lstsq(A, Ly, rcond=None)
    p = sol[0]
    logA = sol[1]
    Acoef = np.exp(logA)
    # predictions for original k
    preds = Acoef * (k ** p)
    # compute R^2 on masked points (log-space)
    preds_masked = Acoef * (k2 ** p)
    ss_res = np.sum((y2 - preds_masked)**2)
    ss_tot = np.sum((y2 - np.mean(y2))**2)
    r2 = 1 - ss_res/ss_tot if ss_tot>0 else 0.0
    return (Acoef, p), preds, r2

# Model feature mapping following o enunciado:
# For SOMA:
#   Hash  -> time ~ a * k + b        (since C = A+B, k_A=k_B=k -> O(k))
#   Tree  -> time ~ a * k * log(k) + b
# For MULT:
#   Hash  -> time ~ a * k^2 + b
#   Tree  -> time ~ a * k^2 * log(k) + b

def feature_matrix(op, estrutura, k_array):
    # returns matrix X (n x 1) feature
    k = np.array(k_array, dtype=float)
    if op == "SOMA":
        if estrutura == "Hash":
            X = k.reshape(-1,1)            # k
        else: # Tree
            # use k*log(k + 1) to avoid log(0). log base e
            X = (k * np.log(1.0 + k)).reshape(-1,1)
    elif op == "MULT":
        if estrutura == "Hash":
            X = (k**2).reshape(-1,1)
        else:
            X = (k**2 * np.log(1.0 + k)).reshape(-1,1)
    else:
        raise ValueError("Operacao desconhecida: " + op)
    return X

# Armazena predições para CSV
pred_rows = []

# Loop por operação e estrutura
ops = df["Operacao"].unique()
for op in ops:
    for estrutura in ["Hash","Tree"]:
        sub = df[(df["Operacao"]==op) & (df["Estrutura"]==estrutura)].copy()
        if sub.empty:
            continue
        sub = sub.sort_values("k")
        k = sub["k"].values
        y = sub["Tempo_ns"].values

        X = feature_matrix(op, estrutura, k)

        # --- Linear fit on designed feature ---
        coeffs, preds, r2 = linear_fit(X, y)
        a = coeffs[0]
        b = coeffs[-1]  # intercept

        # --- Power-law fit as alternative ---
        pl_params, pl_preds, pl_r2 = power_law_fit(k, y)

        # Report
        print("=== {}  |  {} ===".format(op, estrutura))
        print("Linear model (Y = a*feature + b) ; feature =", 
              "k" if (op=="SOMA" and estrutura=="Hash") else
              ("k*log(k)" if op=="SOMA" else ("k^2" if estrutura=="Hash" else "k^2*log(k)")))
        print(" a = {:.6e}, b = {:.6e}, R2 = {:.5f}".format(a, b, r2))
        if pl_params is not None:
            Acoef, p = pl_params
            print("Power-law fit (Y = A * k^p): A = {:.6e}, p = {:.6f}, alt_R2 = {:.5f}".format(Acoef, p, pl_r2))
        print()

        # --- Plots ---
        # 1) measured vs predicted
        plt.figure(figsize=(8,5))
        plt.title(f"{op} - {estrutura}  (measured vs predicted)")
        plt.scatter(k, y, s=10, label="medido")
        plt.plot(k, preds, color='C1', label="predito linear (a*feature + b)")
        if pl_params is not None:
            plt.plot(k, pl_preds, color='C2', linestyle='--', label="predito power-law")
        plt.xlabel("k")
        plt.ylabel("Tempo (ns)")
        plt.legend()
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        fname = os.path.join(OUT_DIR, f"{op}_{estrutura}_measured_vs_pred.png")
        plt.savefig(fname, dpi=200)
        plt.close()

        # 2) residuals
        residuals = y - preds
        plt.figure(figsize=(8,4))
        plt.title(f"{op} - {estrutura}  (resíduos)")
        plt.scatter(k, residuals, s=8)
        plt.axhline(0, color='black', lw=0.8)
        plt.xlabel("k")
        plt.ylabel("Residual (medido - predito) ns")
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        fname = os.path.join(OUT_DIR, f"{op}_{estrutura}_residuals.png")
        plt.savefig(fname, dpi=200)
        plt.close()

        # Append predictions to CSV list
        for i, row in sub.iterrows():
            pred = float(preds[list(sub.index).index(i)])
            pred_rows.append({
                "Operacao": op,
                "Estrutura": estrutura,
                "k": int(row["k"]),
                "Tempo_medido_ns": float(row["Tempo_ns"]),
                "Tempo_predito_linear_ns": pred
            })

# Save predictions CSV
pred_df = pd.DataFrame(pred_rows)
pred_df.to_csv(os.path.join(OUT_DIR, "predicoes_tempo.csv"), index=False)
print("\nResultados e gráficos salvos em:", OUT_DIR)
