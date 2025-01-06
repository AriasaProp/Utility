#!/bin/bash
echo "Hello from Bash in Termux!";
echo ">,<,>=,<=,==,!=";

generate_numbers() {
  local MIN=$1
  local MAX=$2
  local case=$3
  
  for signa in {"","-"}; do
    for signb in {"","-"}; do
      if [[ $case == "bigger" ]]; then
        A=$(shuf -i $((MIN+1))-$MAX -n 1)
        B=$(shuf -i $MIN-$((A-1)) -n 1)
      elif [[ $case == "smaller" ]]; then
        B=$(shuf -i $((MIN+1))-$MAX -n 1)
        A=$(shuf -i $MIN-$((B-1)) -n 1)
      else # equal
        A=$(shuf -i $MIN-$MAX -n 1)
        B=$A
      fi
      echo "${signa}${A}, ${signb}${B}"
    done
  done
}

for i in $(seq 1 3); do
  for j in $(seq 1 $i); do
    MIN=$(( (4294967296 ** (i - j)) - 1 ))
    MAX=$(( (4294967296 ** i) - 1 ))

    # Case: bigger
    generate_numbers $MIN $MAX "bigger"
    # Case: smaller
    generate_numbers $MIN $MAX "smaller"
    # Case: equal
    generate_numbers $MIN $MAX "equal"
  done
done
