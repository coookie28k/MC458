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

# Cores consistentes
COLORS = {
    'Hash': '#1f77b4', # Azul
    'Tree': '#ff7f0e'  # Laranja
}

# ==========================================
# CARREGAMENTO ROBUSTO (CORREÇÃO DO ERRO)
# ==========================================
def load_data_robust(filepath):
    if not os.path.exists(filepath):
        print(f"Erro: '{filepath}' não encontrado.")
        return None

    print(f"Lendo {filepath} e filtrando linhas corrompidas...")
    
    valid_lines = []
    expected_cols = 6 # Operacao,Estrutura,k,Esparsidade,Tempo_ns,Memoria_Bytes
    bad_count = 0
    
    with open(filepath, 'r') as f:
        # Ler cabeçalho
        header = f.readline()
        if not header: return None
        valid_lines.append(header)
        
        for line in f:
            parts = line.strip().split(',')
            # Filtro 1: Número de colunas incorreto (linhas misturadas)
            if len(parts) != expected_cols:
                bad_count += 1
                continue
            
            # Filtro 2: Cabeçalhos repetidos no meio do arquivo
            if parts[0] == 'Operacao':
                continue
                
            # Filtro 3: Linhas que parecem texto misturado (ex: TRANS,Hash...TRANS)
            try:
                # Tenta converter k e Tempo para ver se são números
                int(parts[2]) # k
                float(parts[4]) # Tempo
                valid_lines.append(line)
            except ValueError:
                bad_count += 1
                continue

    print(f"-> Linhas válidas: {len(valid_lines)}")
    print(f"-> Linhas descartadas: {bad_count}")
    
    # Criar DataFrame a partir das strings limpas
    df = pd.read_csv(io.StringIO(''.join(valid_lines)))
    return df

# ==========================================
# TEORIA (EQUAÇÕES)
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

# ==========================================
# PLOTAGEM
# ==========================================
def plot_benchmark():
    df = load_data_robust(INPUT_FILE)
    if df is None or df.empty:
        print("DataFrame vazio ou inválido.")
        return

    # Conversão e Limpeza
    df['Tempo_ns'] = pd.to_numeric(df['Tempo_ns'], errors='coerce')
    df['k'] = pd.to_numeric(df['k'], errors='coerce')
    df = df.dropna(subset=['Tempo_ns', 'k'])
    
    # Converter para ms para ficar legível
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6 

    # Padronizar nomes
    df['Estrutura'] = df['Estrutura'].replace({'Est1(Hash)': 'Hash', 'Est2(Tree)': 'Tree'})

    # Lista de operações
    operacoes = df['Operacao'].unique()

    for op in operacoes:
        # Ignora lixo se houver
        if op not in ['SOMA', 'MULT', 'TRANS', 'ESCALAR', 'SET', 'GET']: continue

        # Cria a figura
        fig, ax = plt.subplots(figsize=(10, 6))
        
        has_data = False
        
        for est in ['Hash', 'Tree']:
            subset = df[(df['Operacao'] == op) & (df['Estrutura'] == est)].sort_values('k')
            if len(subset) < 5: continue # Ignora se tiver poucos pontos
            
            has_data = True

            # Dados Reais
            k_real = subset['k'].values
            y_real = subset['Tempo_ms'].values
            
            # --- REGRESSÃO LINEAR (MMQ) ---
            x_feature, label_latex = get_theoretical_data(op, est, k_real)
            x_matrix = x_feature[:, np.newaxis]
            
            # Calcula slope (a) forçando intercepto 0
            a, _, _, _ = np.linalg.lstsq(x_matrix, y_real, rcond=None)
            slope = a[0]
            
            # Calcula R2
            y_pred = slope * x_feature
            ss_res = np.sum((y_real - y_pred) ** 2)
            ss_tot = np.sum((y_real - np.mean(y_real)) ** 2)
            r2 = 1 - (ss_res / ss_tot) if ss_tot > 1e-9 else 1.0

            # --- PLOTAGEM ---
            color = COLORS[est]
            
            # 1. Pontos Medidos
            ax.scatter(k_real, y_real, color=color, s=20, alpha=0.5, label=f'{est} (Medido)')
            
            # 2. Linha Teórica
            k_smooth = np.linspace(k_real.min(), k_real.max(), 300)
            x_smooth, _ = get_theoretical_data(op, est, k_smooth)
            y_smooth = slope * x_smooth
            
            # Legenda Rica
            label_line = f'{est} Teórico: $y \\approx {slope:.2e} \\cdot {label_latex}$ ($R^2={r2:.3f}$)'
            ax.plot(k_smooth, y_smooth, color=color, linestyle='--', linewidth=2, label=label_line)

        if not has_data:
            plt.close()
            continue

        # Cosmética
        ax.set_title(f'Desempenho em função de k: {op}', fontsize=16, fontweight='bold')
        ax.set_xlabel('Número de Elementos (k)', fontsize=12)
        ax.set_ylabel('Tempo (ms)', fontsize=12)
        
        # Formatação Científica no X
        ax.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
        
        ax.grid(True, alpha=0.3, linestyle='-')
        ax.legend(fontsize=10, loc='upper left', framealpha=0.95)
        
        plt.tight_layout()
        filename = f"benchmark_{op}.png"
        plt.savefig(filename, dpi=300)
        print(f"Gerado: {filename}")
        plt.close()

if __name__ == "__main__":
    sns.set_theme(style=SNS_STYLE)
    plot_benchmark()