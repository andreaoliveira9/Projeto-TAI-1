# Projeto-TAI-1

Primeiro projeto Teoria Algorítmica da Informação

# Finite Context Model (FCM) e Gerador de Texto

Este repositório contém dois programas em C++:

- **fcm**: Calcula o conteúdo médio de informação (em bits por símbolo) de um texto usando um modelo de contexto finito (modelo de Markov).
- **generator**: Gera texto com base num modelo de contexto finito aprendido a partir de um texto de treino.

## Estrutura dos Ficheiros

- `fcm.cpp`: Código fonte do programa fcm.
- `generator.cpp`: Código fonte do programa generator.
- `README.md`: Este ficheiro, que contém instruções de compilação e execução.

## Requisitos

- Compilador C++ com suporte a C++11 (por exemplo, `g++`).
- As bibliotecas padrão do C++ – não são necessárias bibliotecas externas.

## Instruções de Compilação

Pode compilar os programas utilizando os seguintes comandos num ambiente Linux:

```bash
g++ -std=c++11 -o fcm fcm.cpp
g++ -std=c++11 -o generator generator.cpp
```
