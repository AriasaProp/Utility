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
    int ch;
    try {
      simple_timer_t ct;
#define EXTRACT(N) \
if (fscanf(file, " %[^,],",text_buffer) < 1) \
  throw text_buffer; \
Matrix N (text_buffer);
      while((ch = fgetc(file)) != EOF) {
        ++counter;
        switch (ch) {
          case 'A': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
            fgetc(file);
    	      if (A+B != C) throw "+ operator";
      			break;
          }
          case 'B': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
            fgetc(file);
    	      if (A-B != C) throw "- operator";
      			break;
          }
          case 'C': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
            fgetc(file);
    	      if (A*B != C) throw "* operator";
      			break;
          }
          case 'D': {
            EXTRACT(A);
            EXTRACT(B);
            EXTRACT(C);
            fgetc(file);
    	      if (A/B != C) throw "/ operator";
      			break;
          }
          default:
      			throw "unexpected value";
        }
      }
      *output_file << ct.end();
#undef EXTRACT
    } catch (const char *e) {
      *output_file << "Err [" << counter << "] -> " << e;
    } catch (...) {
      *output_file << "Err [" << counter << "] -> unknown";
    }
    fclose(file);
  } else {
    *output_file << "file I/O error! with " << text_buffer;
  }
  *output_file << std::endl;
}