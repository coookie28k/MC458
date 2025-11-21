import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import os

# ==========================================
# CONFIGURAÇÃO
# ==========================================
FILE_NAME = 'resultados_operacoes.csv'  # Nome do seu CSV gerado em C++
SNS_STYLE = "whitegrid"
COLORS = {
    'Est1(Hash)': '#1f77b4',  # ou a cor que você preferir (ex: 'blue')
    'Est2(Tree)': '#ff7f0e',  # ou a cor que você preferir (ex: 'orange')
    'Densa': '#2ca02c'        # (Estou assumindo que a 3ª se chama 'Densa', verifique se não é 'Matriz Densa')
}

# MUITO IMPORTANTE: Faça o mesmo para o dicionário MARKERS se ele existir
MARKERS = {
    'Est1(Hash)': 'o',  # círculo
    'Est2(Tree)': 's',  # quadrado
    'Densa': 'D'        # diamante
}

def load_data():
    if not os.path.exists(FILE_NAME):
        print(f"Erro: Arquivo '{FILE_NAME}' não encontrado.")
        return None
    
    df = pd.read_csv(FILE_NAME)
    
    # Limpeza e Conversão
    df = df[df['Tempo_ns'] > 0] # Remove falhas (-1)
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    
    # Calcular K se não existir (K = N^2 * Esparsidade)
    if 'k' not in df.columns:
        df['k'] = (df['N']**2 * df['Esparsidade']).astype(int)
        
    return df

def plot_comparativo_tempo(df, operacao):
    data = df[df['Operacao'] == operacao]
    if data.empty: return

    plt.figure(figsize=(10, 6))
    
    ax = sns.lineplot(
        data=data, x='k', y='Tempo_ms', hue='Estrutura', style='Estrutura',
        palette=COLORS, markers=MARKERS, dashes=False, linewidth=2.5, markersize=8
    )
    
    plt.title(f'Tempo de Execução: {operacao}', fontsize=14, fontweight='bold')
    plt.xlabel('Número de Elementos Não Nulos (k)', fontsize=12)
    plt.ylabel('Tempo (ms) - Escala Log', fontsize=12)
    
    # Escala Log-Log é essencial para comparar Densa (N^2) com Esparsa (k)
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, which="minor", ls=":", alpha=0.4)
    plt.legend(title='Estrutura')
    
    plt.tight_layout()
    plt.savefig(f'grafico_tempo_{operacao.lower()}.png', dpi=300)
    plt.close()
    print(f"-> Gerado: grafico_tempo_{operacao.lower()}.png")

def plot_comparativo_memoria(df):
    # Usa operação de CONSTRUCAO ou SET como proxy de memória
    data = df[df['Operacao'].isin(['CONSTRUCAO', 'SET'])].copy()
    if data.empty: return
    
    # Pega o maior valor de memória registrado por K para evitar ruído
    data = data.groupby(['Estrutura', 'k'])['Memoria_MB'].max().reset_index()

    plt.figure(figsize=(10, 6))
    
    sns.lineplot(
        data=data, x='k', y='Memoria_MB', hue='Estrutura', style='Estrutura',
        palette=COLORS, markers=MARKERS, dashes=False, linewidth=2.5
    )
    
    plt.title('Consumo de Memória', fontsize=14, fontweight='bold')
    plt.xlabel('Número de Elementos (k)', fontsize=12)
    plt.ylabel('Memória (MB) - Escala Log', fontsize=12)
    
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, which="minor", ls=":", alpha=0.4)
    
    plt.tight_layout()
    plt.savefig('grafico_memoria_comparativo.png', dpi=300)
    plt.close()
    print("-> Gerado: grafico_memoria_comparativo.png")

if __name__ == "__main__":
    sns.set_theme(style=SNS_STYLE)
    df = load_data()
    
    if df is not None:
        # Plotar Memória
        plot_comparativo_memoria(df)
        
        # Plotar Tempo para cada operação disponível
        ops = df['Operacao'].unique()
        for op in ops:
            plot_comparativo_tempo(df, op)