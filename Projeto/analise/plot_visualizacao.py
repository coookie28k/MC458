import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# ==========================================
# CONFIGURAÇÃO
# ==========================================
INPUT_FILE = "resultados_operacoes.csv"
SNS_STYLE = "whitegrid"

# Cores para manter consistência visual (Densa sempre vermelha para destaque)
PALETTE = {
    'Densa': '#d62728',      # Vermelho
    'Est1(Hash)': '#1f77b4', # Azul
    'Est2(Tree)': '#2ca02c', # Verde
    'Hash': '#1f77b4',
    'Tree': '#2ca02c'
}

# Marcadores para diferenciar visualmente os pontos
MARKERS = {
    'Densa': 'X', 
    'Est1(Hash)': 'o', 
    'Est2(Tree)': 's',
    'Hash': 'o',
    'Tree': 's'
}

def plot_comparativo_separado():
    if not os.path.exists(INPUT_FILE):
        print(f"Erro: {INPUT_FILE} não encontrado.")
        return

    # 1. Carregar Dados
    df = pd.read_csv(INPUT_FILE)
    
    # 2. Limpeza e Conversão
    df = df[df['Tempo_ns'] > 0]
    
    # Converter para unidades legíveis
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    
    # 3. Loop por Operação
    operacoes = df['Operacao'].unique()

    for op in operacoes:
        data_op = df[df['Operacao'] == op]
        
        # ==========================================
        # GRÁFICO 1: TEMPO DE EXECUÇÃO
        # ==========================================
        plt.figure(figsize=(8, 6))
        
        sns.lineplot(
            data=data_op, 
            x='N', 
            y='Tempo_ms', 
            hue='Estrutura', 
            style='Estrutura',
            palette=PALETTE,
            markers=MARKERS,
            dashes=False,
            linewidth=2.5,
            markersize=8
        )
        
        plt.title(f'Tempo de Execução: {op}', fontsize=14, fontweight='bold')
        plt.xlabel('Dimensão da Matriz (N)', fontsize=12)
        plt.ylabel('Tempo (ms)', fontsize=12)
        
        # Configurações Logarítmicas
        plt.xscale('log')
        plt.yscale('log')
        plt.grid(True, which="minor", ls=":", alpha=0.4)
        plt.grid(True, which="major", ls="-", alpha=0.8)
        plt.legend(title='Estrutura', fontsize=10)
        
        plt.tight_layout()
        filename_tempo = f"analise_tempo_{op}.png"
        plt.savefig(filename_tempo, dpi=300)
        print(f"Gerado: {filename_tempo}")
        plt.close()

        # ==========================================
        # GRÁFICO 2: CONSUMO DE MEMÓRIA
        # ==========================================
        plt.figure(figsize=(8, 6))
        
        sns.lineplot(
            data=data_op, 
            x='N', 
            y='Memoria_MB', 
            hue='Estrutura', 
            style='Estrutura',
            palette=PALETTE,
            markers=MARKERS,
            dashes=False,
            linewidth=2.5,
            markersize=8
        )
        
        plt.title(f'Consumo de Memória: {op}', fontsize=14, fontweight='bold')
        plt.xlabel('Dimensão da Matriz (N)', fontsize=12)
        plt.ylabel('Memória (MB)', fontsize=12)
        
        # Configurações Logarítmicas
        plt.xscale('log')
        plt.yscale('log')
        plt.grid(True, which="minor", ls=":", alpha=0.4)
        plt.grid(True, which="major", ls="-", alpha=0.8)
        plt.legend(title='Estrutura', fontsize=10)
        
        plt.tight_layout()
        filename_mem = f"analise_memoria_{op}.png"
        plt.savefig(filename_mem, dpi=300)
        print(f"Gerado: {filename_mem}")
        plt.close()

if __name__ == "__main__":
    sns.set_theme(style=SNS_STYLE)
    plot_comparativo_separado()