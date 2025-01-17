#include "hash.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>


#define BYTE_BUFFER_SIZE 2048

bool hash_test (std::ofstream &o, const char *data) {
  o << "Hash Test" << std::endl;
  char buff[2048];
  bool passed = true;
  size_t i, j, k, l, m, n;
  
	o << "SHA256: " << std::endl;
	try {
		strcpy(buff, data);
		strcat(buff, "/data/sha256.txt");
		std::ifstream file(buff);
  	if (!file.is_open()) [[unlikely]] throw "file data not found!";
		try {
	    std::string line, first, second;
	    while (std::getline(file, line)) {
	      std::istringstream ss(line);
	      if (std::getline(ss, first, ',') && std::getline(ss, second, ',')) {
	        hash256 o = sha256 (buff, );
		    	if (memcmp(o.b, e.b, 64)) throw "wrong sha256 hashing result";
	      }
	    }
		} catch (const char *err) {
	  	file.close();
			throw err;
		}
	  file.close();
	} catch (const char *err) {
		passed &= false;
		o << err << std::endl;
	}
  
  
	o << "RandomX: Not yet." << std::endl;
  
	o << "Hash Test End" << std::endl;
	
	return passed;
}