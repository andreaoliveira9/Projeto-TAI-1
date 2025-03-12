# Finite-Context Model (Markov Models) - Fcm and Generator Programs

## Overview
This repository contains two programs:
- `fcm`: A program for performing finite-context modeling.
- `generator`: A program to generate text based on finite-context models.

## Dependencies
To compile and run these programs, ensure you have the following tools installed on your Linux system:

- **g++ Compiler**
- **Make**

Install using the following command:

```bash
sudo apt update
sudo apt install build-essential
```

This command installs `g++`, `make`, and other essential build tools.

## Installation Instructions
Clone this repository and compile the programs using the provided `Makefile`:

```bash
git clone https://github.com/andreaoliveira9/Projeto-TAI-1
cd Projeto-TAI-1
make
```

Alternatively, you can compile the files manually:

```bash
g++ -std=c++17 -o fcm fcm.cpp
g++ -std=c++17 -o generator generator.cpp
```

## Running the Programs

### Running `fcm`

Example command:

```bash
./fcm sequences/sequence1.txt -k 3 -a 0.01
```

- `sequences/sequence1.txt`: Input file.
- `-k 3`: Context size.
- `-a 0.01`: Smoothing parameter (alpha).

The `fcm` program saves the trained model to a file called `model.bin`, which can later be used by the `generator`.

### Running `generator`

The `generator` program can be used in two ways:

1\. Using a specified input file to train a new model:

```bash
./generator -k 3 -a 0.1 -p abc -s 500 -i sequences/sequence1.txt
```

- `-i sequences/sequence1.txt`: Input file to train the model.
- `-k 3`: Context size.
- `-a 0.1`: Smoothing parameter.
- `-p abc`: Initial context (must be equal to or longer than the model's context size).
- `-s 500`: Size of generated text.

2\. Using an existing model (`model.bin`) previously created by `fcm`:

```bash
./generator -k 3 -a 0.1 -p abc -s 500
```

- `-k 3`: Context size (must match the context size used in the trained mode).
- `-a 0.1`: Smoothing parameter (must match the alpha used in the trained model).
- `-p abc`: Initial context (must be equal to or longer than the model's context size).
- `-s 500`: Size of generated text.

## Cleaning

To remove the compiled executables, run:

```bash
make clean
```

