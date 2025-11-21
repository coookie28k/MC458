import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os
import io

# ==========================================
# CONFIGURAÇÃO
# ==========================================
INPUT_FILE = "benchmark.csv"
SNS_STYLE = "whitegrid"
MAX_K = 2000000  # Limite para evitar ruído de memória (ajuste se quiser ver tudo)

# Cores consistentes
COLORS = {
    'Hash': '#1f77b4', # Azul
    'Tree': '#ff7f0e'  # Laranja
}

# ==========================================
# CARREGAMENTO ROBUSTO
# ==========================================
def load_data_robust(filepath):
    if not os.path.exists(filepath):
        print(f"Erro: '{filepath}' não encontrado.")
        return None

    print(f"Lendo {filepath}...")
    
    valid_lines = []
    expected_cols = 6 
    
    with open(filepath, 'r') as f:
        header = f.readline()
        if not header: return None
        valid_lines.append(header)
        
        for line in f:
            parts = line.strip().split(',')
            if len(parts) != expected_cols: continue
            if parts[0] == 'Operacao': continue
            try:
                int(parts[2]) # k
                float(parts[4]) # Tempo
                valid_lines.append(line)
            except ValueError:
                continue

    df = pd.read_csv(io.StringIO(''.join(valid_lines)))
    return df

# ==========================================
# TEORIA
# ==========================================
def get_theoretical_data(op, estrutura, k_values):
    k = np.array(k_values, dtype=float)
    log_k = np.log(np.maximum(k, 1.0)) 
    
    if estrutura == 'Hash':
        if op in ['SOMA', 'SET', 'GET', 'ESCALAR']:
            return k, "k"
        elif op == 'MULT':
            return k**2, "k^2"
        elif op == 'TRANS':
            return np.ones_like(k), "1"
            
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

# ==========================================
# PLOTAGEM
# ==========================================
def plot_benchmark_comparativo():
    df = load_data_robust(INPUT_FILE)
    if df is None or df.empty:
        print("Dados vazios.")
        return

    # Conversão
    df['Tempo_ns'] = pd.to_numeric(df['Tempo_ns'], errors='coerce')
    df['k'] = pd.to_numeric(df['k'], errors='coerce')
    df = df.dropna(subset=['Tempo_ns', 'k'])
    
    # Filtro de K
    df = df[df['k'] <= MAX_K]
    
    # Ms
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6 
    df['Estrutura'] = df['Estrutura'].replace({'Est1(Hash)': 'Hash', 'Est2(Tree)': 'Tree'})

    operacoes = df['Operacao'].unique()

    for op in operacoes:
        if op not in ['SOMA', 'MULT', 'TRANS', 'ESCALAR', 'SET', 'GET']: continue

        fig, ax = plt.subplots(figsize=(10, 6))
        
        # Loop para plotar ambas no mesmo gráfico
        for est in ['Hash', 'Tree']:
            subset = df[(df['Operacao'] == op) & (df['Estrutura'] == est)].sort_values('k')
            if len(subset) < 10: continue 
            
            k_real = subset['k'].values
            y_real = subset['Tempo_ms'].values
            
            # Regressão
            x_feature, label_latex = get_theoretical_data(op, est, k_real)
            x_matrix = x_feature[:, np.newaxis]
            a, _, _, _ = np.linalg.lstsq(x_matrix, y_real, rcond=None)
            slope = a[0]
            
            # R2
            y_pred = slope * x_feature
            ss_res = np.sum((y_real - y_pred) ** 2)
            ss_tot = np.sum((y_real - np.mean(y_real)) ** 2)
            r2 = 1 - (ss_res / ss_tot) if ss_tot > 1e-9 else 1.0

            # Plot Pontos
            color = COLORS[est]
            ax.scatter(k_real, y_real, color=color, s=15, alpha=0.4, label=f'{est} (Medido)')
            
            # Plot Linha Teórica
            k_smooth = np.linspace(k_real.min(), k_real.max(), 300)
            x_smooth, _ = get_theoretical_data(op, est, k_smooth)
            y_smooth = slope * x_smooth
            
            label_line = f'{est} Teo: $y \\approx {slope:.1e} \\cdot {label_latex}$ ($R^2={r2:.3f}$)'
            ax.plot(k_smooth, y_smooth, color=color, linestyle='--', linewidth=2, label=label_line)

        ax.set_title(f'Comparativo: {op}', fontsize=16, fontweight='bold')
        ax.set_xlabel('Número de Elementos (k)', fontsize=12)
        ax.set_ylabel('Tempo (ms)', fontsize=12)
        ax.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=10, loc='upper left', framealpha=0.95)
        
        plt.tight_layout()
        filename = f"comparativo_benchmark_{op}.png"
        plt.savefig(filename, dpi=300)
        print(f"Gerado: {filename}")
        plt.close()

if __name__ == "__main__":
    sns.set_theme(style=SNS_STYLE)
    plot_benchmark_comparativo()