import numpy as np
from collections import Counter
import sys

def shannon_entropy(sequence):
    """
    Compute Shannon entropy H(X) of a sequence.
    """
    freq = Counter(sequence)
    total = len(sequence)
    return -sum((count / total) * np.log2(count / total) for count in freq.values())

def conditional_entropy(sequence, k):
    """Compute conditional entropy H(Y|X) for a given sequence and context length k."""
    context_counts = {}
    total_count = len(sequence) - k  # Total valid contexts

    for i in range(total_count):
        context = sequence[i:i+k]  # Extract k-length context
        symbol = sequence[i+k]  # Next symbol

        if context not in context_counts:
            context_counts[context] = Counter()
        context_counts[context][symbol] += 1

    H_Y_given_X = 0
    for context, symbols in context_counts.items():
        context_prob = sum(symbols.values()) / total_count  # Normalize probability correctly
        entropy_context = -sum((count / sum(symbols.values())) * np.log2(count / sum(symbols.values())) 
                               for count in symbols.values())
        H_Y_given_X += context_prob * entropy_context  # Weighted sum

    return H_Y_given_X

def redundancy(entropy, conditional_entropy):
    """
    Compute redundancy R = 1 - H(Y|X) / H(X)
    """
    return 1 - (conditional_entropy / entropy) if entropy > 0 else 0

# Ensure correct usage
if len(sys.argv) < 2:
    print("Usage: python entropy.py <file_path> -k <order>")
    sys.exit(1)

# Read arguments
file_path = sys.argv[1]
selected_k = int(sys.argv[3]) if len(sys.argv) > 3 and sys.argv[2] == "-k" else 3  # Default k=3

# Read sequence from file
try:
    with open(file_path, "r") as f:
        sequence = f.read().strip()
except FileNotFoundError:
    print(f"Error: File '{file_path}' not found.")
    sys.exit(1)

# Compute entropy metrics
H_X = shannon_entropy(sequence)
H_Y_given_X = conditional_entropy(sequence, selected_k)
R = redundancy(H_X, H_Y_given_X)

# Display results
print(f"File: {file_path}")
print(f"Shannon Entropy (H(X)): {H_X:.4f}")
print(f"Conditional Entropy (H(Y|X)) [k={selected_k}]: {H_Y_given_X:.4f}")
print(f"Redundancy (R): {R:.4f}")