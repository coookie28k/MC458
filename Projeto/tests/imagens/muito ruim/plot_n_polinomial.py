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
    
    # --- FILTROS ---
    # 1. Remover Matriz Densa
    df = df[df['Estrutura'] != 'Densa']
    
    # 2. Converter unidades
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    
    # 3. Limpar dados inválidos
    df = df[df['Tempo_ns'] > 0]
    
    return df

def plot_polinomial_tempo(df, operacao):
    data_op = df[df['Operacao'] == operacao]
    
    if data_op.empty:
        return

    # lmplot com order=2 (Regressão Polinomial Quadrática)
    # Isso se ajusta melhor ao fato de que K cresce com N ao quadrado
    g = sns.lmplot(
        data=data_op,
        x='N',
        y='Tempo_ms',
        hue='Estrutura',
        height=6,
        aspect=1.5,
        order=2,  # <--- A MÁGICA ACONTECE AQUI (Curva em vez de reta)
        ci=None,  # Sem sombra de confiança
        scatter_kws={"s": 80, "alpha": 0.7}, 
        line_kws={"linestyle": "--", "linewidth": 2} 
    )
    
    plt.title(f'Tendência Quadrática (Tempo vs N): {operacao}')
    plt.xlabel('Dimensão da Matriz (N)')
    plt.ylabel('Tempo (ms)')
    
    # Garante que os eixos comecem do zero para noção real de escala
    plt.xlim(left=0)
    plt.ylim(bottom=0)
    
    filename = f'polinomial_N_tempo_{operacao.lower()}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

def plot_polinomial_memoria(df):
    data_mem = df[df['Operacao'] == 'CONSTRUCAO']
    
    g = sns.lmplot(
        data=data_mem,
        x='N',
        y='Memoria_MB',
        hue='Estrutura',
        height=6,
        aspect=1.5,
        order=2, # Memória também cresce quadraticamente se a esparsidade for fixa
        ci=None,
        scatter_kws={"s": 80, "alpha": 0.7},
        line_kws={"linestyle": "--", "linewidth": 2}
    )
    
    plt.title('Tendência Quadrática de Memória (Memória vs N)')
    plt.xlabel('Dimensão da Matriz (N)')
    plt.ylabel('Memória (MB)')
    plt.xlim(left=0)
    plt.ylim(bottom=0)
    
    filename = 'polinomial_N_memoria.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Gerado: {filename}")
    plt.close()

if __name__ == "__main__":
    df = carregar_dados_filtrados()
    
    if df is not None:
        # 1. Memória
        plot_polinomial_memoria(df)
        
        # 2. Operações
        # Nota: Transposta é O(1), então a "curva" deve ser uma reta horizontal perto do zero
        operacoes = ['ESCALAR', 'SOMA', 'MULT', 'TRANS'] 
        for op in operacoes:
            plot_polinomial_tempo(df, op)