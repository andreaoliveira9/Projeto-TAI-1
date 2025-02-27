#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <vector>
#include <cstdlib>

using namespace std;

/*
 * Função que mostra as instruções de utilização do programa fcm.
 */
void printUsage(const char* progName) {
    cout << "Usage: " << progName << " <input_file> -k <order> -a <smoothing_parameter>" << endl;
    cout << "Example: " << progName << " text.txt -k 3 -a 0.01" << endl;
}

/*
 * Função para salvar o modelo (contextCounts) num ficheiro binário.
 * O formato utilizado é o seguinte:
 * 1. Número de contextos (size_t)
 * 2. Para cada contexto:
 *      a) Tamanho da string do contexto (size_t)
 *      b) Os caracteres do contexto
 *      c) Número de entradas no mapa de frequência (size_t)
 *      d) Para cada entrada: o símbolo (char) e a contagem (int)
 */
void saveModelBinary(const unordered_map<string, unordered_map<char, int>>& contextCounts, const string &filename) {
    ofstream out(filename, ios::binary);
    if (!out) {
        cerr << "Erro: Não foi possível abrir o ficheiro " << filename << " para escrita." << endl;
        return;
    }
    
    // Escreve o número de contextos
    size_t numContexts = contextCounts.size();
    out.write(reinterpret_cast<const char*>(&numContexts), sizeof(numContexts));
    
    // Para cada contexto, salva o contexto e as contagens associadas
    for (const auto &pair : contextCounts) {
        const string &context = pair.first;
        // Salva o comprimento do contexto
        size_t contextLength = context.size();
        out.write(reinterpret_cast<const char*>(&contextLength), sizeof(contextLength));
        // Salva os caracteres do contexto
        out.write(context.data(), contextLength);
        
        // Salva o tamanho do mapa de frequências
        const unordered_map<char, int> &freqMap = pair.second;
        size_t mapSize = freqMap.size();
        out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
        
        // Para cada símbolo e sua contagem, escreve no ficheiro
        for (const auto &entry : freqMap) {
            char symbol = entry.first;
            int count = entry.second;
            out.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
            out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        }
    }
    
    out.close();
}

int main(int argc, char* argv[]) {
    // Verifica se foram passados argumentos suficientes
    if (argc != 6) {
        printUsage(argv[0]);
        return 1;
    }

    // O primeiro argumento é o nome do ficheiro de entrada
    string inputFile = argv[1];
    int k = 0;
    double alpha = 0.0;

    // Processa os argumentos da linha de comandos para -k e -a
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-k" && i + 1 < argc) {
            k = stoi(argv[++i]);
        } else if (arg == "-a" && i + 1 < argc) {
            alpha = stod(argv[++i]);
        } else {
            cout << "Parâmetro desconhecido ou incompleto: " << arg << endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Abre o ficheiro de entrada
    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "Erro: Não foi possível abrir o ficheiro " << inputFile << endl;
        return 1;
    }

    // Lê todo o conteúdo do ficheiro para uma string
    stringstream buffer;
    buffer << inFile.rdbuf();
    string text = buffer.str();
    inFile.close();

    // Verifica se o texto tem comprimento suficiente para o valor de k
    if (text.size() <= (size_t)k) {
        cerr << "Erro: O comprimento do texto é menor ou igual à ordem k." << endl;
        return 1;
    }

    // Cria o alfabeto (conjunto dos caracteres únicos presentes no texto)
    unordered_set<char> alphabet;
    for (char c : text) {
        alphabet.insert(c);
    }
    int alphabetSize = alphabet.size();

    // Estrutura para guardar as contagens dos contextos:
    // Chave: string de tamanho k (o contexto)
    // Valor: unordered_map que associa cada símbolo à sua contagem após esse contexto.
    unordered_map<string, unordered_map<char, int>> contextCounts;

    // Percorre o texto a partir da posição k e atualiza as contagens para cada contexto
    for (size_t i = k; i < text.size(); i++) {
        string context = text.substr(i - k, k);
        char symbol = text[i];
        contextCounts[context][symbol]++;
    }

    // Salva o modelo gerado num ficheiro binário
    saveModelBinary(contextCounts, "model.bin");
    // cout << "Modelo salvo em binario no ficheiro modelo.bin" << endl;

    // Calcula o conteúdo total de informação (em bits) para as posições de índice k até o fim do texto.
    double totalInfo = 0.0;
    int countSymbols = 0; // número de símbolos considerados (text.size() - k)

    // Para cada símbolo (a partir de k), calcula a probabilidade do símbolo dado o contexto e soma a informação
    for (size_t i = k; i < text.size(); i++) {
        string context = text.substr(i - k, k);
        char symbol = text[i];
        auto it = contextCounts.find(context);
        if (it == contextCounts.end()) {
            continue;
        }
        int symbolCount = 0;
        int totalCount = 0;
        // Calcula a soma das contagens para o contexto e a contagem específica para o símbolo atual
        for (auto &entry : it->second) {
            totalCount += entry.second;
            if (entry.first == symbol) {
                symbolCount = entry.second;
            }
        }
        // Estima a probabilidade aplicando o parâmetro de suavização (smoothing)
        double probability = (symbolCount + alpha) / (totalCount + alpha * alphabetSize);
        // Calcula o conteúdo de informação deste símbolo: -log2(probabilidade)
        double info = -log2(probability);
        totalInfo += info;
        countSymbols++;
    }

    // Calcula o conteúdo médio de informação (em bits por símbolo)
    double averageInfo = totalInfo / countSymbols;
    cout << averageInfo << endl;

    return 0;
}