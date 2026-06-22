#! /bin/bash -e
echo "" > ht.txt
for x in "" "Hello World" "The quick brown fox jumps over the lazy dog"; do
  (printf $x | sha512sum -t) >> ht.txt
done