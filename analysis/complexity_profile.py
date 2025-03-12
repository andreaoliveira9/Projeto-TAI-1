import subprocess
import itertools
import sys
import matplotlib.pyplot as plt
import os
import pandas as pd
import seaborn as sns

# Apply Seaborn theme
sns.set_theme(style="whitegrid")

# Ensure proper usage
if len(sys.argv) < 2:
    print("Usage: python complexity_profile.py <file_path> [-k <order>] [-a <alpha>]")
    sys.exit(1)

input_file = sys.argv[1]  # File name passed as argument
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

# Define parameter ranges
if selected_k is not None and selected_alpha is not None:
    k_values = [selected_k]
    alpha_values = [selected_alpha]
else:
    k_values = range(1, 11)  # Test k values from 1 to 10
    alpha_values = [0.01, 0.05, 0.1, 0.2, 0.5]

print(f"Running analysis for all k and alpha values...")

# Define the FCM executable
fcm_executable = "./fcm.exe" if os.name == "nt" else "./fcm"

# Ensure the output directory exists
output_dir = "img"
os.makedirs(output_dir, exist_ok=True)

# Dictionary to store results
data = {}

for k in k_values:
    for alpha in alpha_values:
        print(f"Testing k={k}, alpha={alpha}...")
        try:
            # Run the program and capture output
            result = subprocess.run(
                [fcm_executable, input_file, "-k", str(k), "-a", str(alpha)],
                capture_output=True,
                text=True,
            )

            lengths, bps = [], []

            # Extract the BPS values from the output
            output_lines = result.stdout.split("\n")
            for line in output_lines:
                parts = line.strip().split()
                if len(parts) == 3:  # Expecting "<index> <log_probability> <symbol>"
                    lengths.append(int(parts[0]))
                    bps.append(float(parts[1]))

            # Only store data if valid results exist
            if lengths and bps:
                data[(k, alpha)] = (lengths, bps)
            else:
                print(f"Warning: No valid data returned for k={k}, alpha={alpha}")

        except Exception as e:
            print(f"Error testing k={k}, alpha={alpha}: {e}")

# Generate and save plots for all valid (k, alpha) pairs
for (k, alpha), (lengths, bps) in data.items():
    if not lengths or not bps:
        print(f"Skipping plot for k={k}, alpha={alpha} due to lack of valid data.")
        continue  # Skip if no valid data is present

    rolling_window = 5  # Smoothing window size

    plt.figure(figsize=(14, 7))

    # Convert to DataFrame
    df = pd.DataFrame({'Position': lengths, 'Complexity': bps})

    # Apply rolling average
    df["Smoothed"] = df["Complexity"].rolling(window=rolling_window, min_periods=1).mean()

    # Scatter plot
    sns.scatterplot(
        x=df["Position"],
        y=df["Complexity"],
        alpha=0.5,
        color="cyan",
        s=20,
        label="Raw Data"
    )

    # Smoothed trend line
    sns.lineplot(
        x=df["Position"],
        y=df["Smoothed"],
        color="darkblue",
        linewidth=2,
        label="Smoothed Trend"
    )

    # Improve grid styling
    plt.grid(True, linestyle="--", alpha=0.5)

    plt.xlabel("Sequence Position", fontsize=14)
    plt.ylabel("-log(P(e|c)) (Bits per Symbol)", fontsize=14)
    plt.title(f"Sequence Complexity Profile (k={k}, alpha={alpha})", fontsize=16)

    plt.legend(frameon=True, fontsize=12, loc="upper right")

    # Generate filename for saving the plot
    input_filename = os.path.basename(input_file)
    output_filename = f"i_{input_filename}_k_{k}_a_{alpha}.png"
    output_filepath = os.path.join(output_dir, output_filename)

    # Save plot to file
    plt.savefig(output_filepath, dpi=300, bbox_inches="tight")
    print(f"Plot saved as {output_filepath}")

    plt.close()  # Close plot to free memory
