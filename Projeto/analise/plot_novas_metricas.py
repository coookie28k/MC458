import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Configuração estética opcional
plt.style.use('seaborn-v0_8-whitegrid')

# ===============================
# 1. LEITURA E PREPARAÇÃO
# ===============================
# Certifique-se que o nome do arquivo é o gerado pelo C++ (ex: resultados.csv)
input_file = "resultado_reduzido_O3.csv" # Altere para o nome do seu arquivo novo
try:
    df = pd.read_csv(input_file)
except FileNotFoundError:
    print(f"Erro: Arquivo '{input_file}' não encontrado. Verifique o nome.")
    exit()

# Garantir tipos numéricos
cols_num = ["k", "Esparsidade", "Tempo_ns", "Memoria_Bytes"]
for col in cols_num:
    if col in df.columns:
        df[col] = pd.to_numeric(df[col], errors='coerce')

# ===============================
# 2. FUNÇÃO: TEMPO x ESPARSIDADE
# ===============================
def plot_tempo_esparsidade(df, operacao, filename):
    subset = df[df["Operacao"] == operacao].copy()
    
    # Ordenar para o gráfico de linha ficar correto
    subset.sort_values(by="Esparsidade", inplace=True)

    hash_data = subset[subset["Estrutura"] == "Hash"]
    tree_data = subset[subset["Estrutura"] == "Tree"]

    plt.figure(figsize=(10, 6))
    
    plt.plot(hash_data["Esparsidade"], hash_data["Tempo_ns"], "o-", label="Hash", markersize=4)
    plt.plot(tree_data["Esparsidade"], tree_data["Tempo_ns"], "o-", label="Tree", markersize=4)

    plt.title(f"{operacao}: Tempo de Execução vs Esparsidade")
    plt.xlabel("Esparsidade (k / N²)")
    plt.ylabel("Tempo (ns)")
    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)
    
    # Formatar eixo X em notação científica se for muito pequeno
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))

    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    print(f"⚡ Gráfico salvo: {filename}")
    plt.close()

# ===============================
# 3. FUNÇÃO: MEMÓRIA x K
# ===============================
def plot_memoria_k(df, operacao, filename):
    subset = df[df["Operacao"] == operacao].copy()
    subset.sort_values(by="k", inplace=True)

    hash_data = subset[subset["Estrutura"] == "Hash"]
    tree_data = subset[subset["Estrutura"] == "Tree"]

    plt.figure(figsize=(10, 6))
    
    # Converter para MB para facilitar leitura
    plt.plot(hash_data["k"], hash_data["Memoria_Bytes"] / 1e6, "o-", label="Hash", markersize=4)
    plt.plot(tree_data["k"], tree_data["Memoria_Bytes"] / 1e6, "o-", label="Tree", markersize=4)

    plt.title(f"{operacao}: Consumo de Memória vs Elementos (k)")
    plt.xlabel("k (número de elementos não nulos)")
    plt.ylabel("Memória (MB)")
    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)

    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    print(f"⚡ Gráfico salvo: {filename}")
    plt.close()

# ===============================
# 4. EXECUÇÃO DOS PLOTS
# ===============================

if "Esparsidade" in df.columns:
    print("--- Gerando Gráficos de Esparsidade ---")
    plot_tempo_esparsidade(df, "SOMA", "tempo_x_esparsidade_soma.png")
    plot_tempo_esparsidade(df, "MULT", "tempo_x_esparsidade_mult.png")
else:
    print("Aviso: Coluna 'Esparsidade' não encontrada no CSV.")

print("\n--- Gerando Gráficos de Memória ---")
plot_memoria_k(df, "SOMA", "memoria_x_k_soma.png")
# Geralmente memória é igual para SOMA/MULT (depende de k), mas geramos ambos
plot_memoria_k(df, "MULT", "memoria_x_k_mult.png")

print("\n✅ Processo concluído.")