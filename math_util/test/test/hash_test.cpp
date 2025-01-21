#include "hash.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <iostream>

extern std::ostream *output_file;
extern char text_buffer[2048];

bool hash_test (const char *data) {
  *output_file << "Hash Test" << std::endl;
  bool passed = true;
  size_t i, j, k, l, m, n;

  *output_file << "SHA256: " << std::endl;
  try {
    strcpy (text_buffer, data);
    strcat (text_buffer, "/data/sha256.txt");
    FILE *file = fopen (text_buffer, "r");
    if (!file) [[unlikely]]
      throw "file data not found!";
    try {
	    while (fgets(text_buffer, 2048, file)) {
	      char *tok = strchr(text_buffer, ',');
	      hash256 o = sha256 (text_buffer, tok - text_buffer);
	      hash256 e;
      	std::cout << o << std::endl; 
      	std::cout << *(tok+1) << std::endl; 
	      for (i = 0; i < 8; ++i) {
	        for (j = 0; j < 8; ++j) {
	          e.i[i] <<= 4;
	        	++tok;
	          e.i[i] = *tok - 48 + (*tok >= 'a') * 39;
	        }
	      }
      	std::cout << e << std::endl; 
	      if (memcmp (o.b, e.b, 64)) throw "hash result wrong!";
	    }
    	fclose(file);
	  } catch (const char *err) {
	    fclose(file);
	    throw err;
	  }
  } catch (const char *err) {
    passed &= false;
    std::cerr << err << std::endl;
  }

  *output_file << "RandomX: Not yet." << std::endl;
  return passed;
}