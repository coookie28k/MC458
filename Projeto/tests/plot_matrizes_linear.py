import pandas as pd
import matplotlib.pyplot as plt

# ===============================
# LEITURA DO CSV
# ===============================
df = pd.read_csv("old_resultados.csv")

df["k"] = pd.to_numeric(df["k"])
df["Tempo_ns"] = pd.to_numeric(df["Tempo_ns"])
df["Memoria_Bytes"] = pd.to_numeric(df["Memoria_Bytes"])

# ===============================
# FUNÇÃO DE PLOT LINEAR
# ===============================
def plot_metric_linear(df, operacao, coluna, ylabel, filename):
    subset = df[df["Operacao"] == operacao]

    hash_data = subset[subset["Estrutura"] == "Hash"]
    tree_data = subset[subset["Estrutura"] == "Tree"]

    plt.figure(figsize=(10, 6))
    plt.title(f"{operacao}: {ylabel} vs k")
    plt.xlabel("k (quantidade de elementos)")
    plt.ylabel(ylabel)

    # Gráficos lineares
    plt.plot(hash_data["k"], hash_data[coluna], "o-", label="Hash")
    plt.plot(tree_data["k"], tree_data[coluna], "o-", label="Tree")

    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)

    plt.savefig(filename, dpi=300)
    print(f"⚡ Gráfico salvo: {filename}")
    plt.close()


# ===============================
# FUNÇÃO DE PLOT LINEAR
# ===============================
def plot_mult(df, operacao, coluna, ylabel, filename):
    subset = df[df["Operacao"] == operacao]

    hash_data = subset[subset["Estrutura"] == "Hash"]
    tree_data = subset[subset["Estrutura"] == "Tree"]

    plt.figure(figsize=(10, 6))
    plt.title(f"{operacao}: {ylabel} vs Ka . Kb")
    plt.xlabel("Ka . Kb (quantidade de elementos)")
    plt.ylabel(ylabel)

    # Gráficos lineares
    plt.plot(hash_data["k"]*hash_data["k"], hash_data[coluna], "o-", label="Hash")
    plt.plot(tree_data["k"]*tree_data["k"], tree_data[coluna], "o-", label="Tree")

    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)

    plt.savefig(filename, dpi=300)
    print(f"⚡ Gráfico salvo: {filename}")
    plt.close()

# ===============================
# PLOTS DE TEMPO (LINEAR)
# ===============================
plot_metric_linear(df, "SOMA", "Tempo_ns", "Tempo (ns)", "tempo_soma_linear.png")
plot_mult(df, "MULT", "Tempo_ns", "Tempo (ns)", "tempo_mult_linear.png")

# ===============================
# PLOTS DE MEMÓRIA (LINEAR)
# ===============================
plot_metric_linear(df, "SOMA", "Memoria_Bytes", "Memória (bytes)", "memoria_soma_linear.png")
plot_metric_linear(df, "MULT", "Memoria_Bytes", "Memória (bytes)", "memoria_mult_linear.png")

print("\n✅ Gráficos lineares gerados com sucesso!\n")
