import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os


INPUT_FILE = "resultados_operacoes.csv"
SNS_STYLE = "whitegrid"

PALETTE = {
    'Densa': '#d62728',      # Vermelho
    'Est1(Hash)': '#1f77b4', # Azul
    'Est2(Tree)': '#2ca02c', # Verde
    'Hash': '#1f77b4',
    'Tree': '#2ca02c'
}

def plot_memoria_unificado():
    if not os.path.exists(INPUT_FILE):
        print(f"Erro: {INPUT_FILE} não encontrado.")
        return

    # 1. Carregar Dados
    df = pd.read_csv(INPUT_FILE)
    
    # Converter para milisegundos e MB
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    df['k_calculado'] = (df['N']**2 * df['Esparsidade']).astype(int)
    
    # 3. Dados de teste com valor -1 são invalidos
    df = df[df['Memoria_Bytes'] > 0]

    # ===========================================================
    # GRÁFICO 1: Memória x N (AGREGADO)
    # ===========================================================
    df_agregado_n = df.groupby(['Estrutura', 'N'], as_index=False)['Memoria_MB'].mean()

    plt.figure(figsize=(8, 6))
    
    sns.scatterplot(
        data=df_agregado_n, 
        x='N', 
        y='Memoria_MB', 
        hue='Estrutura', 
        style='Estrutura',
        palette=PALETTE,
        s=100,
        alpha=0.9,    
        edgecolor="k"
    )
    
    plt.title('Escalabilidade de Memória por Dimensão (N) - Médias', fontsize=14, fontweight='bold')
    plt.xlabel('Dimensão da Matriz (N)', fontsize=12)
    plt.ylabel('Memória (MB)', fontsize=12)
    
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, which="minor", ls=":", alpha=0.4)
    plt.grid(True, which="major", ls="-", alpha=0.8)
    plt.legend(title='Estrutura', loc='upper left')
    
    plt.tight_layout()
    plt.savefig("analise_memoria_por_N_unificado.png", dpi=300)
    print("Gerado: analise_memoria_por_N_unificado.png")
    plt.close()

    # ===========================================================
    # GRÁFICO 2: Memória x K (AGREGADO - Apenas Esparsas)
    # ===========================================================
    df_esparsas = df[
        (df['Estrutura'].str.contains('Hash|Tree|Est')) & 
        (~df['Estrutura'].str.contains('Densa'))
    ].copy()

    # Agrupa por Estrutura e K
    df_agregado_k = df_esparsas.groupby(['Estrutura', 'k_calculado'], as_index=False)['Memoria_MB'].mean()
    df_agregado_k = df_agregado_k.sort_values('k_calculado')

    plt.figure(figsize=(8, 6))
    
    sns.scatterplot(
        data=df_agregado_k, 
        x='k_calculado', 
        y='Memoria_MB', 
        hue='Estrutura', 
        style='Estrutura',
        palette=PALETTE,
        s=100,
        alpha=0.9,
        edgecolor="k"
    )
    
    plt.title('Eficiência Real (Memória vs Elementos) - Médias', fontsize=14, fontweight='bold')
    plt.xlabel('Número de Elementos Não Nulos (k)', fontsize=12)
    plt.ylabel('Memória (MB)', fontsize=12)
    
    plt.xscale('log') 
    plt.yscale('log') 
    
    plt.grid(True, which="minor", ls=":", alpha=0.4)
    plt.grid(True, which="major", ls="-", alpha=0.8)
    plt.legend(title='Estrutura', loc='upper left')
    
    plt.tight_layout()
    plt.savefig("analise_memoria_por_K_unificado.png", dpi=300)
    print("Gerado: analise_memoria_por_K_unificado.png")
    plt.close()

if __name__ == "__main__":
    sns.set_theme(style=SNS_STYLE)
    plot_memoria_unificado()