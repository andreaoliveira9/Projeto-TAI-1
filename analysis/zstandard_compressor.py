import sys
import os
import zstandard as zstd


def comprimir_ficheiro(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        texto = f.read()

    num_simbolos = len(texto)

    dados_bytes = texto.encode("utf-8")

    compressor = zstd.ZstdCompressor()
    dados_comprimidos = compressor.compress(dados_bytes)

    tamanho_comprimido = len(dados_comprimidos) * 8

    bits_por_simbolo = tamanho_comprimido / num_simbolos if num_simbolos > 0 else 0

    sequence = file_path.split("/")[1].split(".")[0]

    with open("outputs/" + sequence + "_zstandard.txt", "w") as f:
        f.write(f"Tamanho do ficheiro comprimido: {tamanho_comprimido} bits\n")
        f.write(f"Número de símbolos: {num_simbolos}\n")
        f.write(f"Bits por símbolo: {bits_por_simbolo:.4f}")


if __name__ == "__main__":
    sequences = os.listdir("sequences")
    for sequence in sequences:
        comprimir_ficheiro("sequences/" + sequence)
