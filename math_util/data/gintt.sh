#!/bin/bash
echo "Hello from Bash in Termux!";
echo ">,<,>=,<=,==,!=";


for i in $(seq 1 3); do
	for j in $(seq 1 i); do
		MIN=$(((4294967296**(i-j))-1));
		MAX=$(((4294967296**i)-1));
	  #biger
		for signa in {"","-"}; do
			for signb in {"","-"}; do
			  A=$(shuf -i $((MIN+1))-$MAX -n 1);
			  B=$(shuf -i $MIN-$((A-1)) -n 1);
			  echo "${signa}${A}, ${signb}${B}";
			done;
		done;
	  #smaller
		for signa in {"","-"}; do
			for signb in {"","-"}; do
			  B=$(shuf -i $((MIN+1))-$MAX -n 1);
			  A=$(shuf -i $MIN-$((B-1)) -n 1);
			  echo "${signa}${A}, ${signb}${B}";
			done;
		done;
		#equal
		for signa in {"","-"}; do
			for signb in {"","-"}; do
			  A=$(shuf -i $MIN-$MAX -n 1);
			  echo "${signa}${A}, ${signb}${A}";
			done;
		done;
	done;
done;