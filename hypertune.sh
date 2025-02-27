#!/bin/bash

# Declare parameter lists
K_VALUES="3,5,10,15,20"
A_VALUES="0.01,0.05,0.1,0.2,0.5"

# Define the executable and input file
EXECUTABLE="./fcm"
INPUT_FILES="sequence1.txt,sequence2.txt"

# Invoke hyperfine with embedded lists

hyperfine \
    --parameter-list k $K_VALUES \
    --parameter-list a $A_VALUES \
    --parameter-list input_file $INPUT_FILES \
    "./fcm sequences/{input_file} -k {k} -a {a} > outputs/i_{input_file}_k_{k}_a_{a}.txt" --export-json "hypertune.json"
