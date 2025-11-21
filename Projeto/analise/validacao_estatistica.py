import pandas as pd
import numpy as np
import statsmodels.api as sm
import os

FILE_NAME = 'resultados_funcao_k.csv'

# ==============================================================================
# MAPA DE COMPLEXIDADE TEÓRICA
# Define qual função de k explica o comportamento esperado (Feature Teórica)
# ==============================================================================
def get_theoretical_feature(op, estrutura, k_series):
    k = k_series.astype(float)
    # Evitar log(0)
    log_k = np.log(k.replace(0, 1)) 
    
    # --- ESTRUTURA 1: HASH (Baseada em acesso O(1)) ---
    if estrutura == 'Hash':
        if op in ['SET', 'GET', 'ESCALAR', 'SOMA']: 
            return k  # Linear O(k)
        elif op == 'MULT':
            return k**2  # Quadrática O(k^2)
        elif op == 'TRANS':
            return np.ones_like(k) # Constante O(1)
            
    # --- ESTRUTURA 2: TREE (Baseada em acesso O(log k)) ---
    elif estrutura == 'Tree':
        if op in ['SET', 'GET', 'SOMA']:
            return k * log_k  # O(k log k)
        elif op == 'ESCALAR':
            return k  # O(k) - Iteração linear na árvore
        elif op == 'MULT':
            return (k**2) * log_k  # O(k^2 log k)
        elif op == 'TRANS':
            return np.ones_like(k) # Constante O(1)
            
    return k # Default fallback

def run_validation():
    if not os.path.exists(FILE_NAME):
        print("Arquivo não encontrado.")
        return

    df = pd.read_csv(FILE_NAME)
    df = df[df['Tempo_ns'] > 0]
    
    # Se não tiver coluna k, calcula
    if 'k' not in df.columns:
        df['k'] = df['N']**2 * df['Esparsidade']

    # Filtra apenas Hash e Tree (ignora Densa)
    df = df[df['Estrutura'].isin(['Hash', 'Tree', 'Est1(Hash)', 'Est2(Tree)'])]
    
    # Padronizar nomes das estruturas
    df['Estrutura'] = df['Estrutura'].replace({'Est1(Hash)': 'Hash', 'Est2(Tree)': 'Tree'})

    print(f"{'OPERACAO':<10} | {'ESTRUTURA':<6} | {'TEORIA (Feature)':<15} | {'SLOPE (a)':<12} | {'STD ERR':<10} | {'R2 (Ajuste)':<10}")
    print("-" * 85)

    for op in df['Operacao'].unique():
        for est in ['Hash', 'Tree']:
            subset = df[(df['Operacao'] == op) & (df['Estrutura'] == est)].copy()
            if len(subset) < 3: continue # Precisa de pontos mínimos
            
            # 1. Variáveis Y (Tempo) e X (Complexidade Teórica)
            y = subset['Tempo_ns'].values
            x_teorico = get_theoretical_feature(op, est, subset['k']).values
            
            # 2. Regressão Linear SEM Intercepto (y = a*x)
            # O statsmodels OLS por padrão requer add_constant para ter intercepto.
            # Sem add_constant, ele força a reta passar pela origem (0,0).
            model = sm.OLS(y, x_teorico)
            results = model.fit()
            
            # 3. Extrair Métricas
            slope = results.params[0]
            std_err = results.bse[0]
            r2 = results.rsquared
            
            # Nome bonito para a feature teórica
            feat_name = "k"
            if op == 'MULT': feat_name = "k^2" if est == 'Hash' else "k^2 log k"
            elif op == 'TRANS': feat_name = "1 (Const)"
            elif est == 'Tree' and op != 'ESCALAR': feat_name = "k log k"
            
            print(f"{op:<10} | {est:<6} | {feat_name:<15} | {slope:.2e}     | {std_err:.2e}   | {r2:.5f}")

if __name__ == "__main__":
    try:
        run_validation()
        print("\n[INFO] Se R2 > 0.9, o comportamento prático valida a teoria.")
        print("[INFO] 'Slope' representa o tempo médio (ns) por operação elementar.")
    except ImportError:
        print("Erro: Biblioteca 'statsmodels' necessária. Instale com: pip install statsmodels")