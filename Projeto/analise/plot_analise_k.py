import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import os

# Configuração estética para relatórios acadêmicos
sns.set_theme(style="whitegrid")
plt.rcParams.update({
    'font.size': 12,
    'axes.titlesize': 14,
    'axes.labelsize': 12,
    'legend.fontsize': 10,
    'legend.title_fontsize': 12
})

def carregar_dados():
    # Focamos no O3 (Otimizado) pois é o cenário real de produção
    arquivos = ['resultados_construcaoO3.csv', 'resultados_operacoesO3.csv']
    dfs = []
    
    for arq in arquivos:
        if os.path.exists(arq):
            dfs.append(pd.read_csv(arq))
    
    if not dfs:
        print("Erro: Nenhum arquivo CSV O3 encontrado.")
        return None

    df = pd.concat(dfs, ignore_index=True)
    
    # --- ENGENHARIA DE DADOS ---
    # 1. Calcular K (Número de elementos não nulos)
    # K = N * N * Esparsidade (arredondado para int)
    df['K'] = (df['N']**2 * df['Esparsidade']).astype(int)
    
    # 2. Converter unidades
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    
    # 3. Remover falhas (-1) e zeros absolutos que atrapalham log scale
    df = df[df['Tempo_ns'] > 0]
    df = df[df['K'] > 0]
    
    return df

def plot_tempo_vs_k(df, operacao):
    """
    Plota Tempo vs K.
    Isso mostra: "Para processar K elementos, quanto tempo leva?"
    """
    data_op = df[df['Operacao'] == operacao]
    
    if data_op.empty:
        return

    plt.figure(figsize=(10, 6))
    
    # Plot principal
    sns.lineplot(
        data=data_op, 
        x='K', 
        y='Tempo_ms', 
        hue='Estrutura', 
        style='Estrutura',
        markers=True, 
        dashes=False, 
        linewidth=2.5,
        err_style=None # Remove sombra de erro para limpar visual
    )
    
    plt.title(f'Escalabilidade em função de K: {operacao}')
    plt.xlabel('Número de Elementos Não-Nulos (K)')
    plt.ylabel('Tempo (ms) - Escala Log')
    
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, which="minor", ls=":", alpha=0.4)
    plt.grid(True, which="major", ls="-", alpha=0.8)
    
    filename = f'analise_K_tempo_{operacao.lower()}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

def plot_memoria_vs_k(df):
    """
    Plota Memória vs K.
    A prova definitiva de O(k) vs O(N^2).
    """
    # Usamos 'CONSTRUCAO' como proxy para uso de memória estática da estrutura
    data_mem = df[df['Operacao'] == 'CONSTRUCAO']
    
    plt.figure(figsize=(10, 6))
    
    sns.lineplot(
        data=data_mem, 
        x='K', 
        y='Memoria_MB', 
        hue='Estrutura', 
        style='Estrutura',
        markers=True, 
        dashes=False, 
        linewidth=2.5,
        err_style=None
    )
    
    plt.title('Eficiência de Memória em função de K')
    plt.xlabel('Número de Elementos Não-Nulos (K)')
    plt.ylabel('Memória (MB) - Escala Log')
    
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, which="minor", ls=":", alpha=0.4)
    
    filename = 'analise_K_memoria.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

def plot_crossover_ponto_virada(df, n_fixo=1000):
    """
    Foca em um N específico para mostrar ONDE a Densa ganha da Esparsa.
    Eixo X é Esparsidade (%) em vez de K absoluto, para facilitar leitura.
    """
    # Filtra N fixo e operação de SOMA (onde geralmente ocorre a disputa)
    data_fixed = df[(df['N'] == n_fixo) & (df['Operacao'] == 'SOMA')]
    
    if data_fixed.empty:
        print(f"Sem dados para N={n_fixo} na Soma.")
        return

    plt.figure(figsize=(10, 6))
    
    # Converter esparsidade para porcentagem
    data_fixed = data_fixed.copy()
    data_fixed['Porcentagem'] = data_fixed['Esparsidade'] * 100
    
    sns.lineplot(
        data=data_fixed,
        x='Porcentagem',
        y='Tempo_ms',
        hue='Estrutura',
        style='Estrutura',
        markers=True,
        linewidth=3,
        markersize=10
    )
    
    plt.title(f'Ponto de Virada (Cross-over) na Soma (Matriz {n_fixo}x{n_fixo})')
    plt.xlabel('Densidade da Matriz (%)')
    plt.ylabel('Tempo (ms)')
    
    # Escala Linear aqui é melhor para ver o cruzamento das linhas
    plt.grid(True)
    
    # Anotação visual
    plt.annotate('Zona de Vantagem\nEsparsa', xy=(1, data_fixed['Tempo_ms'].min()), 
                 xytext=(0.5, data_fixed['Tempo_ms'].max()*0.5),
                 arrowprops=dict(facecolor='black', shrink=0.05),
                 fontsize=10)
    
    filename = f'analise_crossover_N{n_fixo}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

if __name__ == "__main__":
    df = carregar_dados()
    
    if df is not None:
        # 1. Gráficos de Memória (Prova do requisito de memória)
        plot_memoria_vs_k(df)
        
        # 2. Gráficos de Tempo por Operação (Escalar e Soma são os mais reveladores)
        operacoes_foco = ['ESCALAR', 'SOMA', 'CONSTRUCAO', 'MULT']
        for op in operacoes_foco:
            plot_tempo_vs_k(df, op)
            
        # 3. O Gráfico "Matador": Onde a Densa vence?
        # Tenta plotar para N=1000 se existir nos dados
        plot_crossover_ponto_virada(df, n_fixo=1000)