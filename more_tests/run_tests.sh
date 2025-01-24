#!/bin/bash

# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Directories
input_dir="./input"
output_dir="./output"
result_dir="./result"

# Create the result direcrory if if doesn't exist
mkdir -p "$result_dir"

# Loop through all input files in the input directory
for input_file in "$input_dir"/t*.in; do
	# Extract the base name without directory and extension
	base_name=$(basename "$input_file" .in)

	# Define corresponding output and result files
	expected_output_file="$output_dir/$base_name.out"
	result_file="$result_dir/$base_name.res"

	# Run the executable with the input file, and write the output to the result file
	./hw5 < "$input_file" | lli > "$result_file"

	# Compare the result file with the expected output file
	if diff -q "$result_file" "$expected_output_file" > /dev/null; then
		echo -e "[${GREEN}PASS${NC}] $base_name"
	else
		echo -e "[${RED}FAIL${NC}] $base_name"
	fi
done
