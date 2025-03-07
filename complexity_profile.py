import subprocess
import itertools
import sys
import matplotlib.pyplot as plt
import os

# Verifica se o nome do ficheiro foi passado como argumento
if len(sys.argv) < 2:
    print("Usage: python complexity_profile.py <file_path> -k <order> -a <alpha>")
    sys.exit(1)

input_file = sys.argv[1]  # Nome do ficheiro passado como argumento
selected_k = None
selected_alpha = None

# Parse command-line arguments
try:
    for i in range(2, len(sys.argv)):
        if sys.argv[i] == "-k" and i + 1 < len(sys.argv):
            selected_k = int(sys.argv[i + 1])
        elif sys.argv[i] == "-a" and i + 1 < len(sys.argv):
            selected_alpha = float(sys.argv[i + 1])
except ValueError:
    print("Invalid k or alpha value. Please provide numeric values.")
    sys.exit(1)

# Set default values if not provided
if selected_k is None:
    selected_k = 7  # Default k value
if selected_alpha is None:
    selected_alpha = 0.01  # Default alpha value

print(f"Running analysis with k={selected_k}, alpha={selected_alpha}")

# Configuração do intervalo de parâmetros
k_values = range(1, 11)  # Testar valores de k de 3 a 9
alpha_values = [0.01, 0.05, 0.1, 0.2, 0.5]  # Novos valores de alpha


fcm_executable = "./fcm.exe" if os.name == "nt" else "./fcm"


# Dictionary to store results for each (k, alpha) pair
data = {}


colors = itertools.cycle(["b", "g", "r", "c", "m", "y", "k"])

# Testar todas as combinações de k e alpha
for k, alpha in itertools.product(k_values, alpha_values):
    print(f"Testando k={k}, alpha={alpha}...")
    try:
        # Executar o programa e capturar a saída
        result = subprocess.run(
            [fcm_executable, input_file, "-k", str(k), "-a", str(alpha)],
            capture_output=True,
            text=True,
        )

        lengths = []
        bps = []

        # Extrair o valor de BPS da saída
        output_lines = result.stdout.split("\n")
        for line in output_lines:
            parts = line.strip().split()  # Splitting by spaces/tabs
            if len(parts) == 3:  # Expecting "<index> <log_probability> <symbol>"
                lengths.append(int(parts[0]))  # Store the sequence position (index)
                bps.append(float(parts[1]))  # Store -log(P(e|c)) (bits per symbol)

        # Store data for this (k, alpha)
        data[(k, alpha)] = (lengths, bps)

        # Plotar os resultados para o valor atual de k
        color = next(colors)
        plt.scatter(lengths, bps, s=1, label=f"k={k}", color=color)


    except Exception as e:
        print(f"Erro ao testar k={k}, alpha={alpha}: {e}")



# Show combined plot of all (k, alpha) values
plt.xlabel("Sequence Position")
plt.ylabel("-log(P(e|c)) (Bits per Symbol)")
plt.title("Sequence Complexity Profile (All k, alpha combinations)")
plt.legend()
plt.show()

# Plot only for the selected (k, alpha) combination
plt.figure(figsize=(12, 6))

if (selected_k, selected_alpha) in data:
    filtered_lengths, filtered_bps = data[(selected_k, selected_alpha)]
    plt.scatter(filtered_lengths, filtered_bps, s=1, color="blue", alpha=0.5)
else:
    print(f"No data found for k={selected_k}, alpha={selected_alpha}")

plt.xlabel("Sequence Position")
plt.ylabel("-log(P(e|c)) (Bits per Symbol)")
plt.title(f"Sequence Complexity Profile (k={selected_k}, alpha={selected_alpha})")
plt.show()