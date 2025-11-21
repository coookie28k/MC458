import pandas as pd
import matplotlib.pyplot as plt

# ===============================
# CONFIGURAÇÕES GERAIS
# ===============================
plt.rcParams['figure.figsize'] = (10, 6)
plt.rcParams['font.size'] = 12

# ===============================
# LEITURA DO CSV
# ===============================
df = pd.read_csv("resultados.csv")

# Converte k para numérico (garantia)
df["k"] = pd.to_numeric(df["k"])
df["Tempo_ns"] = pd.to_numeric(df["Tempo_ns"])
df["Memoria_Bytes"] = pd.to_numeric(df["Memoria_Bytes"])

# ===============================
# FUNÇÃO DE PLOT
# ===============================
def plot_metric(df, operacao, coluna, ylabel, filename):
    subset = df[df["Operacao"] == operacao]

    hash_data = subset[subset["Estrutura"] == "Hash"]
    tree_data = subset[subset["Estrutura"] == "Tree"]

    plt.figure()
    plt.title(f"{operacao}: {ylabel} vs k")
    plt.xlabel("k (quantidade de elementos)")
    plt.ylabel(ylabel)

    # Escala log-log para capturar tendência assintótica
    plt.xscale("log")
    plt.yscale("log")

    # Linhas
    plt.plot(hash_data["k"], hash_data[coluna], "o-", label="Hash")
    plt.plot(tree_data["k"], tree_data[coluna], "o-", label="Tree")

    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)

    plt.savefig(filename, dpi=300)
    print(f"⚡ Gráfico salvo: {filename}")
    plt.close()

# ===============================
# PLOTS DE TEMPO
# ===============================
plot_metric(df, "SOMA", "Tempo_ns", "Tempo (ns)", "tempo_soma.png")
plot_metric(df, "MULT", "Tempo_ns", "Tempo (ns)", "tempo_mult.png")

# ===============================
# PLOTS DE MEMÓRIA
# ===============================
plot_metric(df, "SOMA", "Memoria_Bytes", "Memória (bytes)", "memoria_soma.png")
plot_metric(df, "MULT", "Memoria_Bytes", "Memória (bytes)", "memoria_mult.png")

print("\n✅ Todos os gráficos foram gerados com sucesso!\n")
