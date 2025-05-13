#include "math/Matrix.hpp"
#include "simple_profiling.hpp"

#include <ostream>
#include <cstdio>

extern std::ostream *output_file;
extern char *text_buffer;
extern const char *data_address;

void Matrix_test () {
  *output_file << "Matrix: ";
  sprintf (text_buffer, "%s/MatrixTest.txt", data_address);
  FILE *file = fopen(text_buffer, "r");
  if (file) {
    size_t counter = 0;
    unsigned int c, r, j, i;
    int ch;
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
    }
    fclose(file);
  } else {
    *output_file << "file I/O error! with " << text_buffer;
  }
  *output_file << std::endl;
}