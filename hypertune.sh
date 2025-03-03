#!/bin/bash

# Declare parameter lists
K_VALUES="3,5,10,15,20"
A_VALUES="0.01,0.05,0.1,0.2,0.5"

# Define the executable and input file
EXECUTABLE="./fcm"
INPUT_FILES="sequence1.txt,sequence2.txt,sequence3.txt,sequence5.txt"

# Invoke hyperfine with embedded lists

hyperfine --max-runs 3 \
    --parameter-list k $K_VALUES \
    --parameter-list a $A_VALUES \
    --parameter-list input_file $INPUT_FILES \
    'bash -c "p=\$(head -c {k} sequences/{input_file}); \
    output1=\$(./fcm sequences/{input_file} -k {k} -a {a}); \
    ./generator -k {k} -a {a} -p \"\$p\" -s 500 > outputs/temp.txt; \
    output2=\$(./fcm outputs/temp.txt -k {k} -a {a}); \
    printf \"%s,%s\n\" \"\$output1\" \"\$output2\" >> outputs/i_{input_file}_k_{k}_a_{a}.txt"' \
    --export-json "hypertune.json"