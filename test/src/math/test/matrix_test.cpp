#include "math/Matrix.hpp"
#include "simple_profiling.hpp"

#include <ostream>
#include <cstdio>

extern std::ostream *output_file;

bool Matrix_test () {
  bool passed = true;
  *output_file << "Matrix: ";
  FILE *file = fopen("data/math/MatrixTest.txt", "r");
  if (file) {
    size_t counter = 0;
    unsigned int c, r, j, i;
    int ch;
    char *src = (char*)malloc(4096);
    try {
#define EXTRACT(N) \
  if (fscanf(file, "[%u %u]", &c, &r) < 2) \
    throw "error read text p"; \
  j = c*r; \
  double *data##N = (double*)malloc(sizeof(double)*c*r); \
  for(i = 0; i < j; ++i) \
    if (!fscanf(file, "%lf", data##N + i)) throw "error read text d"; \
  fscanf(file, "}"); \
  Matrix N (c,r, data##N)
      simple_timer_t ct;
      while((ch = fgetc(file)) != EOF) {
        ++counter;
        switch (ch) {
          case 'A': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
    	      if (A+B != C)
  			  		throw "+ operator";
      			break;
          }
          case 'B': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
    	      if (A-B != C)
  			  		throw "- operator";
      			break;
          }
          case 'C': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
    	      if (A*B != C)
  			  		throw "* operator";
      			break;
          }
          case 'D': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
    	      if (A/B != C)
  			  		throw "/ operator";
      			break;
          }
          default:
      			throw "unexpected value";
        }
      }
#undef EXTRACT
      *output_file << ct.end();
    } catch (const char *e) {
      *output_file << "Err " << counter << " -> " << e;
      passed = false;
    }
    fclose(file);
    free(src);
  } else {
    *output_file << "file I/O error!";
    passed = false;
  }
  *output_file << std::endl;
  return passed;
}