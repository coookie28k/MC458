import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os


INPUT_FILE = "resultados_funcao_k.csv"
SNS_STYLE = "whitegrid"

COLORS = {
    'Hash': '#1f77b4', # Azul
    'Tree': '#ff7f0e'  # Laranja
}

# ==========================================
# LÓGICA DE COMPLEXIDADE (TEORIA)
# ==========================================
def get_theoretical_data(op, estrutura, k_values):
    k = np.array(k_values, dtype=float)
    log_k = np.log(np.maximum(k, 1.0)) 
    
    # --- HASH ---
    if estrutura == 'Hash':
        if op in ['SOMA', 'SET', 'GET', 'ESCALAR']:
            return k, "k"
        elif op == 'MULT':
            return k**2, "k^2"
        elif op == 'TRANS':
            return np.ones_like(k), "1"
            
    # --- TREE ---
    elif estrutura == 'Tree':
        if op in ['SOMA', 'SET', 'GET']:
            return k * log_k, "k \\log k"
        elif op == 'ESCALAR':
            return k, "k" 
        elif op == 'MULT':
            return (k**2) * log_k, "k^2 \\log k"
        elif op == 'TRANS':
            return np.ones_like(k), "1"
            
    return k, "k"

def plot_combinado_ms():
    if not os.path.exists(INPUT_FILE):
        print(f"Erro: {INPUT_FILE} não encontrado.")
        return

    df = pd.read_csv(INPUT_FILE)
    
    # Dados de teste com valor -1 são invalidos e linhas corrompidas
    df = df[df['Tempo_ns'] > 0]
    df['k'] = pd.to_numeric(df['k'], errors='coerce')
    df['Tempo_ns'] = pd.to_numeric(df['Tempo_ns'], errors='coerce')
    df = df.dropna(subset=['k', 'Tempo_ns'])

    # --- CONVERSÃO PARA MILISSEGUNDOS (ms) ---
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6 

    # Padronizar nomes para diferentes arquivos de teste
    df['Estrutura'] = df['Estrutura'].replace({'Est1(Hash)': 'Hash', 'Est2(Tree)': 'Tree'})
    operacoes = df['Operacao'].unique()

    for op in operacoes:
        if op not in ['SOMA', 'MULT', 'TRANS', 'ESCALAR', 'SET', 'GET']: continue

        # Cria a figura
        _, ax = plt.subplots(figsize=(10, 6))
        
        # Loop pelas estruturas para plotar no MESMO gráfico
        for est in ['Hash', 'Tree']:
            subset = df[(df['Operacao'] == op) & (df['Estrutura'] == est)].sort_values('k')
            
            if subset.empty: continue

            # Dados Reais (Agora em ms)
            k_real = subset['k'].values
            y_real = subset['Tempo_ms'].values # <--- Usando MS
            
            # --- REGRESSÃO ---
            x_feature, label_latex = get_theoretical_data(op, est, k_real)
            x_matrix = x_feature[:, np.newaxis]
            a, _, _, _ = np.linalg.lstsq(x_matrix, y_real, rcond=None)
            slope = a[0]
            
            # R2 Score
            y_pred_stats = slope * x_feature
            ss_res = np.sum((y_real - y_pred_stats) ** 2)
            ss_tot = np.sum((y_real - np.mean(y_real)) ** 2)
            r2 = 1 - (ss_res / ss_tot) if ss_tot > 1e-9 else 1.0

            # --- PLOTAGEM ---
            color = COLORS[est]
            
            # 1. Pontos Medidos (Scatter)
            ax.scatter(k_real, y_real, color=color, s=30, alpha=0.6, 
                       label=f'{est} (Medido)')
            
            # 2. Linha Teórica (Curva Suave)
            k_smooth = np.linspace(k_real.min(), k_real.max(), 300)
            x_smooth, _ = get_theoretical_data(op, est, k_smooth)
            y_smooth = slope * x_smooth
            
            # Label da linha com a equação (Slope agora é ms/op)
            label_line = f'{est} Teórico: $y \\approx {slope:.1e} \\cdot {label_latex}$ ($R^2={r2:.3f}$)'
            
            ax.plot(k_smooth, y_smooth, color=color, linestyle='--', linewidth=2, 
                    label=label_line)

        # Decoração Final do Gráfico
        ax.set_title(f'Comparativo de Desempenho: {op}', fontsize=16, fontweight='bold')
        ax.set_xlabel('Número de Elementos (k)', fontsize=12)
        ax.set_ylabel('Tempo (ms)', fontsize=12) # <--- Label atualizado
        
        # Formatação Científica apenas no eixo X (k), Y já está em ms (legível)
        ax.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
        
        ax.grid(True, alpha=0.3, linestyle='-')
        ax.legend(fontsize=10, loc='upper left', frameon=True, framealpha=0.9)
        
        plt.tight_layout()
        
        filename = f"comparativo_ms_{op}.png"
        plt.savefig(filename, dpi=300)
        print(f"Gerado: {filename}")
        plt.close()

if __name__ == "__main__":
    sns.set_theme(style=SNS_STYLE)
    plot_combinado_ms()