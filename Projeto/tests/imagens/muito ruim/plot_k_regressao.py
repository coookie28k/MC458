import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Configuração estética
sns.set_theme(style="whitegrid")
plt.rcParams.update({
    'font.size': 12,
    'axes.titlesize': 14,
    'axes.labelsize': 12
})

def carregar_dados_filtrados():
    # Focamos no O3 (Otimizado)
    arquivos = ['resultados_construcaoO3.csv', 'resultados_operacoesO3.csv']
    dfs = []
    
    for arq in arquivos:
        if os.path.exists(arq):
            dfs.append(pd.read_csv(arq))
    
    if not dfs:
        print("Erro: Nenhum arquivo CSV O3 encontrado.")
        return None

    df = pd.concat(dfs, ignore_index=True)
    
    # --- FILTRO CRÍTICO ---
    # 1. Remover Matriz Densa (Conforme solicitado)
    df = df[df['Estrutura'] != 'Densa']
    
    # 2. Calcular K
    df['K'] = (df['N']**2 * df['Esparsidade']).astype(int)
    
    # 3. Converter unidades
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    
    # 4. Limpar dados inválidos
    df = df[df['Tempo_ns'] > 0]
    df = df[df['K'] > 0]
    
    return df

def plot_regressao_tempo(df, operacao):
    data_op = df[df['Operacao'] == operacao]
    
    if data_op.empty:
        return

    # lmplot cria Scatter + Linha de Regressão
    g = sns.lmplot(
        data=data_op,
        x='K',
        y='Tempo_ms',
        hue='Estrutura',
        height=6,
        aspect=1.5,
        ci=None, # Remove a sombra de intervalo de confiança (mais limpo)
        scatter_kws={"s": 80, "alpha": 0.7}, # Config dos pontos
        line_kws={"linestyle": "--", "linewidth": 2} # Config da linha (tracejada)
    )
    
    # Ajustes de Títulos e Eixos (Escala Linear é o padrão)
    plt.title(f'Tendência Linear (Tempo vs K): {operacao}')
    plt.xlabel('Número de Elementos (K)')
    plt.ylabel('Tempo (ms)')
    plt.xlim(left=0) # Começar do zero para ver a origem
    plt.ylim(bottom=0)
    
    filename = f'regressao_K_tempo_{operacao.lower()}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

def plot_regressao_memoria(df):
    data_mem = df[df['Operacao'] == 'CONSTRUCAO'] # Proxy para memória
    
    g = sns.lmplot(
        data=data_mem,
        x='K',
        y='Memoria_MB',
        hue='Estrutura',
        height=6,
        aspect=1.5,
        ci=None,
        scatter_kws={"s": 80, "alpha": 0.7},
        line_kws={"linestyle": "--", "linewidth": 2}
    )
    
    plt.title('Tendência Linear de Memória (Memória vs K)')
    plt.xlabel('Número de Elementos (K)')
    plt.ylabel('Memória (MB)')
    plt.xlim(left=0)
    plt.ylim(bottom=0)
    
    filename = 'regressao_K_memoria.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

if __name__ == "__main__":
    df = carregar_dados_filtrados()
    
    if df is not None:
        # 1. Memória
        plot_regressao_memoria(df)
        
        # 2. Operações Principais (Escalar e Soma mostram bem a linearidade)
        operacoes = ['ESCALAR', 'SOMA', 'MULT']
        for op in operacoes:
            plot_regressao_tempo(df, op)