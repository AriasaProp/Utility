#include "math/BigInteger.hpp"
#include "util/simple_clock.hpp"

#include <cmath>
#include <cstdlib>
#include <ostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <cstdio>
#include <string>

extern std::ostream *output_file;

bool BigInteger_test () {
  *output_file << "BigInteger Test: ";
  char *src = (char*)malloc(4096);

  bool passed = true;
  simple_timer_t ct;
  
  // do calculation
  FILE *file = fopen("data/math/BigIntegerTest.txt", "r");
  if (file) {
		size_t cnt= 0;
    try {
  		ct.start();
  		int ch = 0;
      while((ch = fgetc(file)) != EOF) {
        cnt++;
      	switch (ch) {
      		case 'A': {
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger B(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A+B != C)
  			  		throw src;
      			break;
      		}
      		case 'B': {
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger B(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A-B != C)
  			  		throw src;
      			break;
      		}
      		case 'C': {
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger B(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if ((A*B) != C)
    		  		throw src;
      			break;
      		}
      		case 'D': {
      		  if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger B(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A/B != C) {
  			  	  *output_file << (A/B) << " " << C;
  			  		throw src;
  			  	}
      			break;
      		}
      		case 'E': {
      		  if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      	    int B;
      	    if (!fscanf(file, " %d", &B))
      	      throw "format test wrong!";
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A.pow(B) != C)
  			  		throw src;
      			break;
      		}
      		case 'F': {
      		  if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A.sqrt() != C)
  			  		throw src;
      			break;
      		}
      		case 'G': {
      		  if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger A(src);
      		  if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger B(src);
      	    if (!fscanf(file, " %s", src))
      	      throw "format test wrong!";
      	    BigInteger C(src);
      	    if (!fscanf(file, " #%[^\n]", src))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A%B != C)
  			  		throw src;
      			break;
      		}
      		default:
      			throw "unexpected value";
      	}
      }
      *output_file << ct.end();
    } catch (const char *msg) {
      *output_file << "Error " << cnt << " -> " << msg;
      passed = false;
    }
    fclose(file);
  } else {
    *output_file << "file I/O error!";
    passed = false;
  }
  *output_file << std::endl;
  free(src);
  return passed;
}