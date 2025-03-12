import subprocess
import itertools
import sys

# Verifica se o nome do ficheiro foi passado como argumento
if len(sys.argv) < 2:
    print("Uso: python sequences_fcm_tuning.py <caminho_do_ficheiro>")
    sys.exit(1)

input_file = sys.argv[1]  # Nome do ficheiro passado como argumento

# Configuração do intervalo de parâmetros
k_values = range(1, 11)  # Testar valores de k de 3 a 9
alpha_values = [
    1 / (10**n) for n in range(1, 11)
]  # Testar valores de alpha de 0.1 a 0.0000000001

fcm_executable = "./fcm"  # Executável do programa

# Variáveis para armazenar o melhor resultado
best_k = None
best_alpha = None
best_bps = float("inf")

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
            if (
                "bits por símbolo" in line.lower()
            ):  # Procurar linha com "bits por símbolo"
                bps_value = float(line.split()[-4])

                # Atualizar melhor configuração
                if bps_value < best_bps:
                    best_bps = bps_value
                    best_k = k
                    best_alpha = alpha
                break
    except Exception as e:
        print(f"Erro ao testar k={k}, alpha={alpha}: {e}")

subprocess.run(
    [fcm_executable, input_file, "-k", str(best_k), "-a", str(best_alpha)],
    capture_output=True,
    text=True,
)
# Exibir os melhores parâmetros encontrados
print(f"Melhores parâmetros: k={best_k}, alpha={best_alpha}, bps={best_bps}")
