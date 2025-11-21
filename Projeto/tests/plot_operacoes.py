import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Configuração visual
sns.set_theme(style="whitegrid")
plt.rcParams.update({'font.size': 12})

def plotar_operacoes(nivel_otimizacao):
    # Arquivos de entrada
    file_construcao = f'resultados_construcao{nivel_otimizacao}.csv'
    file_operacoes = f'resultados_operacoes{nivel_otimizacao}.csv'
    
    if not os.path.exists(file_construcao) or not os.path.exists(file_operacoes):
        print(f"Arquivos para {nivel_otimizacao} não encontrados. Pulando.")
        return

    # 1. Carregar e Unificar Dados
    df1 = pd.read_csv(file_construcao)
    df2 = pd.read_csv(file_operacoes)
    df = pd.concat([df1, df2], ignore_index=True)

    # 2. Limpeza e Conversão
    # Remover falhas (-1)
    df = df[df['Tempo_ns'] > 0]
    
    # Converter unidades
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    df['Memoria_MB'] = df['Memoria_Bytes'] / 1e6
    
    # Lista de operações únicas
    operacoes = df['Operacao'].unique()

    # 3. Gerar Gráficos para cada Operação
    for op in operacoes:
        data_op = df[df['Operacao'] == op]
        
        # Criar figura com 2 subplots (Tempo e Memória)
        fig, axes = plt.subplots(1, 2, figsize=(16, 6))
        fig.suptitle(f'Desempenho: {op} ({nivel_otimizacao})', fontsize=16, fontweight='bold')

        # --- Gráfico de Tempo ---
        sns.lineplot(
            data=data_op, x='N', y='Tempo_ms', hue='Estrutura', style='Estrutura',
            markers=True, dashes=False, linewidth=2.5, ax=axes[0]
        )
        axes[0].set_title('Tempo de Execução (Log Scale)')
        axes[0].set_ylabel('Tempo (ms)')
        axes[0].set_xscale('log')
        axes[0].set_yscale('log') # Essencial para ver a diferença Densa vs Esparsa
        axes[0].grid(True, which="both", ls="-", alpha=0.5)

        # --- Gráfico de Memória ---
        sns.lineplot(
            data=data_op, x='N', y='Memoria_MB', hue='Estrutura', style='Estrutura',
            markers=True, dashes=False, linewidth=2.5, ax=axes[1]
        )
        axes[1].set_title('Consumo de Memória (Log Scale)')
        axes[1].set_ylabel('Memória (MB)')
        axes[1].set_xscale('log')
        axes[1].set_yscale('log')
        axes[1].grid(True, which="both", ls="-", alpha=0.5)

        plt.tight_layout()
        
        # Salvar
        filename = f'grafico_{op.lower()}_{nivel_otimizacao}.png'
        plt.savefig(filename)
        print(f"Gerado: {filename}")
        plt.close()

# Rodar para ambos os níveis
if __name__ == "__main__":
    print("--- Gerando Gráficos O0 (Sem Otimização) ---")
    plotar_operacoes("O0")
    
    print("\n--- Gerando Gráficos O3 (Otimizado) ---")
    plotar_operacoes("O3")