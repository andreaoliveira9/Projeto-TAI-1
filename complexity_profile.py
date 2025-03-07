import subprocess
import itertools
import sys
import matplotlib.pyplot as plt
import os
import pandas as pd
import seaborn as sns


# Apply Seaborn theme
sns.set_theme(style="whitegrid")

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

# Define parameter ranges only if k and alpha were not provided
if selected_k is not None and selected_alpha is not None:
    k_values = [selected_k]
    alpha_values = [selected_alpha]
else:
    k_values = range(1, 11)
    alpha_values = [0.01, 0.05, 0.1, 0.2, 0.5]

print(f"Running analysis with k={selected_k}, alpha={selected_alpha}")

fcm_executable = "./fcm.exe" if os.name == "nt" else "./fcm"


# Dictionary to store results for each (k, alpha) pair
data = {}


colors = itertools.cycle(["b", "g", "r", "c", "m", "y", "k"])

for k in k_values:
    for alpha in alpha_values:
        print(f"Testing k={k}, alpha={alpha}...")
        try:
            # Executar o programa e capturar a saída
            result = subprocess.run(
                [fcm_executable, input_file, "-k", str(k), "-a", str(alpha)],
                capture_output=True,
                text=True,
            )

            lengths, bps = [], []

            # Extrair o valor de BPS da saída
            output_lines = result.stdout.split("\n")
            for line in output_lines:
                parts = line.strip().split()  # Splitting by spaces/tabs
                if len(parts) == 3:  # Expecting "<index> <log_probability> <symbol>"
                    lengths.append(int(parts[0]))  # Store the sequence position (index)
                    bps.append(float(parts[1]))  # Store -log(P(e|c)) (bits per symbol)

            # Store data for this (k, alpha)
            data[(k, alpha)] = (lengths, bps)

        except Exception as e:
            print(f"Erro ao testar k={k}, alpha={alpha}: {e}")


# Plot results only if user provided values
if selected_k is not None and selected_alpha is not None:

    # Set rolling window size (adjust as needed)
    rolling_window = 5
    # Plot only for the selected (k, alpha) combination
    plt.figure(figsize=(14, 7))

    if (selected_k, selected_alpha) in data:

        filtered_lengths, filtered_bps = data[(selected_k, selected_alpha)]

        # Convert to DataFrame for easy processing
        df = pd.DataFrame({'Position': filtered_lengths, 'Complexity': filtered_bps})
        
       # Apply rolling average for smoothing
        df["Smoothed"] = df["Complexity"].rolling(window=rolling_window, min_periods=1).mean()

       # **Scatter plot improvements**
        sns.scatterplot(
            x=df["Position"], 
            y=df["Complexity"], 
            alpha=0.1,  # **Lower transparency**
            color="cyan",  
            s=5,  # **Smaller point size**
            label="Raw Data"
        )

        # **Smoothed trend line improvements**
        sns.lineplot(
            x=df["Position"], 
            y=df["Smoothed"], 
            color="darkblue",  
            linewidth=2,  
            label="Smoothed Trend"
        )
        
    else:
        print(f"No data found for k={selected_k}, alpha={selected_alpha}")

    # **Improve grid styling**
    plt.grid(True, linestyle="--", alpha=0.5)

    plt.xlabel("Sequence Position", fontsize=14)
    plt.ylabel("-log(P(e|c)) (Bits per Symbol)", fontsize=14)
    plt.title(f"Sequence Complexity Profile (k={selected_k}, alpha={selected_alpha})", fontsize=16)
    
    # **Improve legend positioning**
    plt.legend(frameon=True, fontsize=12, loc="upper right")
    plt.show()