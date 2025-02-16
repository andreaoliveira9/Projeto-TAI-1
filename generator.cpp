#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <cmath>
#include <cstdlib>

using namespace std;

/*
 * Função que mostra as instruções de utilização do programa generator.
 */
void printUsage(const char* progName) {
    cout << "Usage: " << progName << " [input_file] -k <order> -a <smoothing_parameter> -p <prior> -s <size>" << endl;
    cout << "Se input_file não for fornecido, o programa lê o texto de treino a partir da entrada padrão." << endl;
    cout << "Example: " << progName << " training.txt -k 3 -a 0.1 -p abc -s 500" << endl;
    cout << "         " << progName << " -k 3 -a 0.1 -p abc -s 500   (lê da entrada padrão)" << endl;
}

int main(int argc, char* argv[]) {
    // Verifica se foram passados argumentos mínimos
    if (argc < 5) {
        printUsage(argv[0]);
        return 1;
    }

    string inputFile = "";
    int argIndex = 1;
    // Se o primeiro argumento não começar com '-' consideramos que é o ficheiro de treino
    if (argv[argIndex][0] != '-') {
        inputFile = argv[argIndex];
        argIndex++;
    }

    int k = 0;
    double alpha = 0.0;
    string prior = "";
    int genSize = 0;

    // Processa os argumentos da linha de comandos
    while (argIndex < argc) {
        string arg = argv[argIndex];
        if (arg == "-k" && argIndex + 1 < argc) {
            k = stoi(argv[++argIndex]);
        } else if (arg == "-a" && argIndex + 1 < argc) {
            alpha = stod(argv[++argIndex]);
        } else if (arg == "-p" && argIndex + 1 < argc) {
            prior = argv[++argIndex];
        } else if (arg == "-s" && argIndex + 1 < argc) {
            genSize = stoi(argv[++argIndex]);
        } else {
            cout << "Parâmetro desconhecido ou incompleto: " << arg << endl;
            printUsage(argv[0]);
            return 1;
        }
        argIndex++;
    }

    // Lê o texto de treino a partir do ficheiro (se fornecido) ou da entrada padrão
    string trainingText;
    if (!inputFile.empty()) {
        ifstream inFile(inputFile);
        if (!inFile) {
            cerr << "Erro: Não foi possível abrir o ficheiro " << inputFile << endl;
            return 1;
        }
        stringstream buffer;
        buffer << inFile.rdbuf();
        trainingText = buffer.str();
        inFile.close();
    } else {
        // Lê a entrada padrão
        string line;
        while (getline(cin, line)) {
            trainingText += line + "\n";
        }
    }

    // Verifica se o texto de treino tem comprimento suficiente para o valor de k
    if (trainingText.size() <= (size_t)k) {
        cerr << "Erro: O comprimento do texto de treino é menor ou igual à ordem k." << endl;
        return 1;
    }

    // Cria o alfabeto (conjunto dos caracteres únicos presentes no texto de treino)
    unordered_set<char> alphabet;
    for (char c : trainingText) {
        alphabet.insert(c);
    }
    int alphabetSize = alphabet.size();

    // Estrutura para guardar as contagens do modelo de contexto finito aprendido
    unordered_map<string, unordered_map<char, int>> contextCounts;

    // Constrói o modelo usando o texto de treino
    for (size_t i = k; i < trainingText.size(); i++) {
        string context = trainingText.substr(i - k, k);
        char symbol = trainingText[i];
        contextCounts[context][symbol]++;
    }

    // Verifica se o argumento "prior" tem pelo menos k caracteres
    if (prior.size() < (size_t)k) {
        cerr << "Erro: O parâmetro prior deve ter pelo menos " << k << " caracteres." << endl;
        return 1;
    }
    // Utiliza apenas os últimos k caracteres do prior como contexto inicial
    string currentContext = prior.substr(prior.size() - k, k);

    // O texto gerado começa com o prior
    string generatedText = prior;

    // Configura o gerador de números aleatórios
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    // Gera "genSize" símbolos adicionais
    for (int i = 0; i < genSize; i++) {
        // Determina a distribuição de probabilidade para o próximo símbolo dado o contexto atual
        double totalCount = 0.0;
        // Para garantir que consideramos todos os símbolos do alfabeto (mesmo os nunca vistos no contexto)
        vector<char> symbols;
        vector<double> probabilities;
        for (char symbol : alphabet) {
            int count = 0;
            // Se o contexto atual existe no modelo e o símbolo foi visto, obtém a contagem
            if (contextCounts.find(currentContext) != contextCounts.end() &&
                contextCounts[currentContext].find(symbol) != contextCounts[currentContext].end()) {
                count = contextCounts[currentContext][symbol];
            }
            totalCount += (count + alpha);
            symbols.push_back(symbol);
        }
        // Calcula a distribuição cumulativa para amostrar o próximo símbolo
        vector<double> cumulative;
        double sumProb = 0.0;
        for (char symbol : symbols) {
            int count = 0;
            if (contextCounts.find(currentContext) != contextCounts.end() &&
                contextCounts[currentContext].find(symbol) != contextCounts[currentContext].end()) {
                count = contextCounts[currentContext][symbol];
            }
            double prob = (count + alpha); // Dividiremos por totalCount depois.
            sumProb += prob;
            cumulative.push_back(sumProb);
        }
        // Normaliza a distribuição cumulativa para o intervalo [0, 1]
        for (double &val : cumulative) {
            val /= totalCount;
        }
        // Gera um número aleatório e seleciona o símbolo com base na distribuição cumulativa
        double r = dis(gen);
        char nextSymbol = symbols.back(); // valor padrão (último símbolo) se ocorrer algum problema de arredondamento
        for (size_t j = 0; j < cumulative.size(); j++) {
            if (r <= cumulative[j]) {
                nextSymbol = symbols[j];
                break;
            }
        }
        // Acrescenta o símbolo selecionado ao texto gerado
        generatedText.push_back(nextSymbol);
        // Atualiza o contexto atual (janela deslizante de tamanho k)
        currentContext = generatedText.substr(generatedText.size() - k, k);
    }

    // Imprime o texto gerado
    cout << generatedText << endl;

    return 0;
}