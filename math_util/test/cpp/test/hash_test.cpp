#include "hash.hpp"
#include "simple_clock.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <iostream>

extern std::ostream *output_file;
extern char text_buffer[2048];
extern const char *data_address;
extern void sha256d(char *, const char *, int);
bool hash_test () {
  bool passed = true;

  *output_file << "Hash Test\nSHA256: ";
  strcpy (text_buffer, data_address);
  strcat (text_buffer, "/sha256.txt");
  FILE *file = fopen (text_buffer, "r");
  if (file) {
    while (passed && fgets(text_buffer, 2048, file)) {
      char *tok = strchr(text_buffer, ',');
      hash256 o = sha256 (text_buffer, tok - text_buffer);
      hash256 e = hash256FromString(tok + 1);
      passed = !memcmp (o.b, e.b, 32);
    }
  	fclose(file);
  	*output_file << (passed ? "Success" : "hash result wrong!");
  } else {
    *output_file << "file data not found!";
    passed & false;
  }
  *output_file << std::endl;

  *output_file << "RandomX: Not yet." << std::endl;
  return passed;
}