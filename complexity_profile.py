import subprocess
import itertools
import sys
import matplotlib.pyplot as plt

# Verifica se o nome do ficheiro foi passado como argumento
if len(sys.argv) < 2:
    print("Uso: python sequences_fcm_tuning.py <caminho_do_ficheiro>")
    sys.exit(1)

input_file = sys.argv[1]  # Nome do ficheiro passado como argumento

# Configuração do intervalo de parâmetros
k_values = range(5, 11)  # Testar valores de k de 3 a 9
alpha_values = [
    1 / (10**n) for n in range(2, 3)
]  # Testar valores de alpha de 0.1 a 0.0000000001

fcm_executable = "./fcm"  # Executável do programa

lengths = []
bps = []

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

        # Extrair o valor de BPS da saída
        output_lines = result.stdout.split("\n")
        for line in output_lines:
            if "len:info" in line.lower():  # Procurar linha com "bits por símbolo"
                values = line.split()
                lengths.append(int(values[1]))
                bps.append(float(values[2]))

        # Plotar os resultados para o valor atual de k
        color = next(colors)
        plt.scatter(lengths[:100000], bps[:100000], s=1, label=f"k={k}", color=color)

    except Exception as e:
        print(f"Erro ao testar k={k}, alpha={alpha}: {e}")

plt.xlabel("Tamanho da sequência")
plt.ylabel("Bits por símbolo")
plt.title("Complexidade de sequências FCM com filtro de baixo passo")
plt.xlim(0)
plt.ylim(0)
plt.legend()
plt.show()
