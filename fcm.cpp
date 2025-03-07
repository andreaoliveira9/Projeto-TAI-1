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

// Função para exibir instruções de uso
void printUsage(const string& progName) {
    cout << "Usage: " << progName << " <input_file> -k <order> -a <smoothing_parameter>" << endl;
    cout << "Example: " << progName << " text.txt -k 3 -a 0.01" << endl;
}

// Função para salvar o modelo binário
void saveModelBinary(const unordered_map<string, unordered_map<char, int>>& contextCounts, const string& filename) {
    ofstream out(filename, ios::binary);
    if (!out) {
        cerr << "Erro: Não foi possível abrir o ficheiro " << filename << " para escrita." << endl;
        return;
    }
    
    size_t numContexts = contextCounts.size();
    out.write(reinterpret_cast<const char*>(&numContexts), sizeof(numContexts));
    
    for (const auto& [context, freqMap] : contextCounts) {
        size_t contextLength = context.size();
        out.write(reinterpret_cast<const char*>(&contextLength), sizeof(contextLength));
        out.write(context.data(), contextLength);
        
        size_t mapSize = freqMap.size();
        out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
        
        for (const auto& [symbol, count] : freqMap) {
            out.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
            out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        }
    }
    
    out.close();
}

// Função para calcular o conteúdo médio de informação
double calculateAverageInformation(const unordered_map<string, unordered_map<char, int>>& contextCounts, const string& text, int k, double alpha) {
    unordered_set<char> alphabet(text.begin(), text.end());
    int alphabetSize = alphabet.size();
    
    double totalInfo = 0.0;
    int countSymbols = text.size() - k;
    
    for (size_t i = k; i < text.size(); i++) {
        string context = text.substr(i - k, k);
        char symbol = text[i];
        
        auto it = contextCounts.find(context);
        if (it == contextCounts.end()) continue;
        
        int totalCount = 0;
        int symbolCount = it->second.at(symbol);
        
        for (const auto& [_, count] : it->second) {
            totalCount += count;
        }
        
        double probability = (   + alpha) / (totalCount + alpha * alphabetSize);
        double logProb = -log2(probability);

        totalInfo += logProb;
    
        // Output per-symbol probability for plotting
        cout << i << " " << logProb << " " << symbol << endl;
    }
    
    return totalInfo / countSymbols;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printUsage(argv[0]);
        return 1;
    }
    
    string inputFile = argv[1];
    int k = 0;
    double alpha = 0.0;
    
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-k" && i + 1 < argc) {
            k = stoi(argv[++i]);
        } else if (arg == "-a" && i + 1 < argc) {
            alpha = stod(argv[++i]);
        } else {
            cerr << "Parâmetro desconhecido ou incompleto: " << arg << endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "Erro: Não foi possível abrir o ficheiro " << inputFile << endl;
        return 1;
    }
    
    stringstream buffer;
    buffer << inFile.rdbuf();
    string text = buffer.str();
    inFile.close();
    
    if (text.size() <= static_cast<size_t>(k)) {
        cerr << "Erro: O comprimento do texto é menor ou igual à ordem k." << endl;
        return 1;
    }
    
    unordered_map<string, unordered_map<char, int>> contextCounts;
    for (size_t i = k; i < text.size(); i++) {
        contextCounts[text.substr(i - k, k)][text[i]]++;
    }
    
    saveModelBinary(contextCounts, "model.bin");
    cout << "Modelo salvo em binário no ficheiro model.bin" << endl;
    
    double averageInfo = calculateAverageInformation(contextCounts, text, k, alpha);
    cout << "Conteúdo Médio de Informação: " << averageInfo << " bits por símbolo" << endl;
    
    return 0;
}