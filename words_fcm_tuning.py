import subprocess
import re
import sys

# Verifica se o nome do ficheiro foi passado como argumento
if len(sys.argv) < 2:
    print("Uso: python words_fcm_tuning.py <caminho_do_ficheiro>")
    sys.exit(1)

input_file = sys.argv[1]  # Nome do ficheiro passado como argumento


# Função para calcular o tamanho médio das palavras
def calcular_tamanho_medio_palavras(arquivo):
    with open(arquivo, "r", encoding="utf-8") as f:
        texto = f.read()
    palavras = re.findall(r"\b\w+\b", texto)
    tamanho_medio = (
        round(sum(len(p) for p in palavras) / len(palavras)) if palavras else 5
    )
    return max(1, tamanho_medio)  # Garante que k seja pelo menos 1


# Configuração do intervalo de parâmetros
alpha_values = [
    1 / (10**n) for n in range(1, 11)
]  # Testar valores de alpha de 0.1 a 0.0000000001

k = calcular_tamanho_medio_palavras(
    input_file
)  # Define k com base no tamanho médio das palavras
fcm_executable = "./fcm"  # Executável do programa

# Variáveis para armazenar o melhor resultado
best_alpha = None
best_bps = float("inf")

# Testar todas as combinações de alpha para o k calculado
for alpha in alpha_values:
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
                    best_alpha = alpha
                break
    except Exception as e:
        print(f"Erro ao testar k={k}, alpha={alpha}: {e}")

subprocess.run(
    [fcm_executable, input_file, "-k", str(k), "-a", str(best_alpha)],
    capture_output=True,
    text=True,
)
# Exibir os melhores parâmetros encontrados
print(f"Melhores parâmetros: k={k}, alpha={best_alpha}, bps={best_bps}")
