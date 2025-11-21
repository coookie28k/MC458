import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

sns.set_theme(style="whitegrid")

def plotar_set_get(nivel_otimizacao):
    arquivo = f'resultados_insercao_consulta{nivel_otimizacao}.csv'
    
    if not os.path.exists(arquivo):
        print(f"Arquivo {arquivo} não encontrado.")
        return

    df = pd.read_csv(arquivo)
    
    # Conversão
    df['Tempo_ms'] = df['Tempo_ns'] / 1e6
    
    # Separar SET e GET
    ops = ['SET', 'GET']
    
    for op in ops:
        data_op = df[df['Operacao'] == op]
        
        plt.figure(figsize=(10, 6))
        
        sns.lineplot(
            data=data_op, x='Ops', y='Tempo_ms', hue='Estrutura', style='Estrutura',
            markers=True, dashes=False, linewidth=2.5
        )
        
        plt.title(f'Escalabilidade: {op} ({nivel_otimizacao})', fontsize=15)
        plt.xlabel('Quantidade de Operações')
        plt.ylabel('Tempo Total (ms)')
        
        # Aqui escala linear geralmente é melhor para ver a inclinação (O(1) vs O(log n))
        # Mas se a diferença for brutal, descomente a linha abaixo:
        # plt.yscale('log') 
        
        plt.grid(True)
        plt.tight_layout()
        
        filename = f'grafico_{op.lower()}_{nivel_otimizacao}.png'
        plt.savefig(filename)
        print(f"Gerado: {filename}")
        plt.close()

if __name__ == "__main__":
    plotar_set_get("O0")
    plotar_set_get("O3")