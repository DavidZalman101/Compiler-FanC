#!/usr/bin/bash

input_dir="./input"
result_dir="./result"

for input_file in "$input_dir"/t*.in; do
	
	base_name=$(basename "$input_file" .in)
	result_file="$result_dir/$base_name.res"

	./hw5 < "$input_file" | lli > "$result_file"
done
