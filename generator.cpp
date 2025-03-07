#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

using namespace std;

// Função para exibir instruções de uso
template <typename T>
void printUsage(const T &progName)
{
    cout << "Usage: " << progName << " -k <order> -a <smoothing_parameter> -p <prior> -s <size> [-i <input_file>]" << endl;
    cout << "Exemplo: " << progName << " -k 3 -a 0.1 -p abc -s 500 [-i sequences/sequence2.txt]" << endl;
}

// Função para carregar o modelo salvo em um arquivo binário
void loadModelBinary(unordered_map<string, unordered_map<char, int>> &contextCounts, const string &filename)
{
    ifstream in(filename, ios::binary);
    if (!in)
    {
        cerr << "Erro: Não foi possível abrir o ficheiro " << filename << " para leitura." << endl;
        return;
    }

    size_t numContexts;
    in.read(reinterpret_cast<char *>(&numContexts), sizeof(numContexts));

    for (size_t i = 0; i < numContexts; i++)
    {
        size_t contextLength;
        in.read(reinterpret_cast<char *>(&contextLength), sizeof(contextLength));

        string context(contextLength, ' ');
        in.read(&context[0], contextLength);

        size_t mapSize;
        in.read(reinterpret_cast<char *>(&mapSize), sizeof(mapSize));
        unordered_map<char, int> freqMap;

        for (size_t j = 0; j < mapSize; j++)
        {
            char symbol;
            int count;
            in.read(reinterpret_cast<char *>(&symbol), sizeof(symbol));
            in.read(reinterpret_cast<char *>(&count), sizeof(count));
            freqMap[symbol] = count;
        }
        contextCounts[context] = freqMap;
    }
    in.close();
}

// Função para processar argumentos da linha de comando
bool processArguments(int argc, char *argv[], int &k, double &alpha, string &prior, int &genSize, string &inputFile)
{
    if (argc != 9 && argc != 11)
    {
        printUsage(argv[0]);
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-k" && i + 1 < argc)
        {
            k = stoi(argv[++i]);
        }
        else if (arg == "-a" && i + 1 < argc)
        {
            alpha = stod(argv[++i]);
        }
        else if (arg == "-p" && i + 1 < argc)
        {
            prior = argv[++i];
        }
        else if (arg == "-s" && i + 1 < argc)
        {
            genSize = stoi(argv[++i]);
        }
        else if (arg == "-i" && i + 1 < argc)
        {
            inputFile = argv[++i];
        }
        else
        {
            cerr << "Parâmetro desconhecido ou incompleto: " << arg << endl;
            printUsage(argv[0]);
            return false;
        }
    }
    return true;
}

// Função para gerar um modelo de previsão baseado no texto de entrada
void buildModel(unordered_map<string, unordered_map<char, int>> &contextCounts, const string &trainingText, int k)
{
    for (size_t i = k; i < trainingText.size(); i++)
    {
        string context = trainingText.substr(i - k, k);
        char symbol = trainingText[i];
        contextCounts[context][symbol]++;
    }
}

int levenshteinDistance(const string &s1, const string &s2)
{
    int len1 = s1.size(), len2 = s2.size();
    vector<vector<int>> dp(len1 + 1, vector<int>(len2 + 1));

    for (int i = 0; i <= len1; i++)
        dp[i][0] = i;
    for (int j = 0; j <= len2; j++)
        dp[0][j] = j;

    for (int i = 1; i <= len1; i++)
    {
        for (int j = 1; j <= len2; j++)
        {
            if (s1[i - 1] == s2[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                dp[i][j] = min({dp[i - 1][j - 1], dp[i - 1][j], dp[i][j - 1]}) + 1;
            }
        }
    }
    return dp[len1][len2];
}

string findClosestContext(const unordered_map<string, unordered_map<char, int>> &contextCounts,
                          const string &targetContext, mt19937 &rng)
{
    int minDistance = 10;
    vector<string> closestMatches;
    vector<int> matchWeights;

    for (const auto &entry : contextCounts)
    {
        int dist = levenshteinDistance(targetContext, entry.first);
        int weight = 0;

        for (const auto &freq : entry.second)
        {
            weight += freq.second;
        }

        if (dist < minDistance)
        {
            minDistance = dist;
            closestMatches = {entry.first};
            matchWeights = {weight};
        }
        else if (dist == minDistance)
        {
            closestMatches.push_back(entry.first);
            matchWeights.push_back(weight);
        }
    }
    
    if (closestMatches.size() == 1)
    {
        return closestMatches[0];
    }

    discrete_distribution<int> dist(matchWeights.begin(), matchWeights.end());
    return closestMatches[dist(rng)];
}

template <typename RNG>
string generateText(const unordered_map<string, unordered_map<char, int>> &contextCounts, const string &prior, int genSize, double alpha, int k, RNG &rng)
{
    string generatedText = prior;
    unordered_set<char> alphabet;
    for (const auto &pair : contextCounts)
    {
        for (const auto &entry : pair.second)
        {
            alphabet.insert(entry.first);
        }
    }
    int alphabetSize = alphabet.size();
    uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < genSize; i++)
    {
        string currentContext = generatedText.substr(generatedText.size() - k, k);
        if (contextCounts.find(currentContext) == contextCounts.end())
        {
            currentContext = findClosestContext(contextCounts, currentContext, rng);
            if (currentContext.empty())
                break; // If no similar context exists, stop
        }

        unordered_map<char, double> probabilities;
        double sum = 0.0;
        for (const auto &entry : contextCounts.at(currentContext))
        {
            sum += entry.second;
        }

        for (const auto &entry : contextCounts.at(currentContext))
        {
            probabilities[entry.first] = (entry.second + alpha) / (sum + alpha * alphabetSize);
        }

        double randomValue = dis(rng);
        double cumulativeSum = 0.0;
        char nextChar = '\0';
        for (const auto &entry : probabilities)
        {
            cumulativeSum += entry.second;
            if (randomValue <= cumulativeSum)
            {
                nextChar = entry.first;
                break;
            }
        }
        if (nextChar == '\0' && !probabilities.empty())
        {
            nextChar = probabilities.begin()->first;
        }
        generatedText += nextChar;
    }
    return generatedText;
}

int main(int argc, char *argv[])
{
    int k;
    double alpha;
    string prior;
    int genSize;
    string inputFile;

    if (!processArguments(argc, argv, k, alpha, prior, genSize, inputFile))
    {
        return 1;
    }

    unordered_map<string, unordered_map<char, int>> contextCounts;
    if (!inputFile.empty())
    {
        ifstream inFile(inputFile);
        if (!inFile)
        {
            cerr << "Erro: Não foi possível abrir o ficheiro " << inputFile << endl;
            return 1;
        }
        stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();

        if (buffer.str().size() <= (size_t)k)
        {
            cerr << "Erro: O comprimento do texto de treino é menor ou igual à ordem k." << endl;
            return 1;
        }
        buildModel(contextCounts, buffer.str(), k);
    }
    else
    {
        loadModelBinary(contextCounts, "model.bin");
        if (contextCounts.empty())
        {
            cerr << "Erro: O modelo não foi carregado corretamente." << endl;
            return 1;
        }
    }

    random_device rd;
    mt19937 rng(rd());
    cout << generateText(contextCounts, prior, genSize, alpha, k, rng) << endl;

    return 0;
}
