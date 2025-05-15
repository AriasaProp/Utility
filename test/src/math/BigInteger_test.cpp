#include "math/BigInteger.hpp"
#include "simple_profiling.hpp"

#include <cmath>
#include <cstdlib>
#include <ostream>
#include <iomanip>
#include <cstdio>
#include <string>
#include <cerrno>
#include <cstring>

// undef TIME to reach limit of file digits
#define TIME 0.5

extern std::ostream *output_file;
extern char *text_buffer;
extern const char *data_address;

void BigInteger_test () {
  *output_file << "BigInteger Test:\n";
  // basic operation test
  *output_file << "  - basic: ";
  simple_timer_t ct;
  
  bool passed = true;
  sprintf(text_buffer, "%s/BigIntegerTest.txt", data_address);
  FILE *file = fopen(text_buffer, "r");
  if (file) {
	  size_t cnt= 0;
    try {
  		ct.start();
  		int ch = 0;
      while((ch = fgetc(file)) != EOF) {
        cnt++;
#define EXTRACT(N) \
if (!fscanf(file, " %s", text_buffer)) \
  throw "format test wrong!"; \
BigInteger N(text_buffer)
      	switch (ch) {
      		case 'A': {
      	    EXTRACT(A);
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A+B != C)
  			  		throw text_buffer;
      			break;
      		}
      		case 'B': {
      	    EXTRACT(A);
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A-B != C)
  			  		throw text_buffer;
      			break;
      		}
      		case 'C': {
      	    EXTRACT(A);
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if ((A*B) != C)
    		  		throw text_buffer;
      			break;
      		}
      		case 'D': {
      	    EXTRACT(A);
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A/B != C)
  			  		throw text_buffer;
      			break;
      		}
      		case 'E': {
      	    EXTRACT(A);
      	    size_t B;
      	    if (!fscanf(file, " %zd", &B))
      	      throw "format test wrong!";
      	    EXTRACT(C);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if ((A^B) != C)
  			  		throw text_buffer;
      			break;
      		}
      		case 'F': {
      	    EXTRACT(A);
      	    EXTRACT(B);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A.sqrt() != B)
  			  		throw text_buffer;
      			break;
      		}
      		case 'G': {
      		  EXTRACT(A);
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A%B != C)
  			  		throw text_buffer;
      			break;
      		}
      		case 'H': {
      		  EXTRACT(A);
      		  BigInteger A1 = A;
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    EXTRACT(D);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A1.div_mod(B) != C || (A1 != D))
  			  		throw text_buffer;
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
    } catch (...) {
      *output_file << "Error unknown!";
      passed = false;
    }
    fclose(file);
  } else {
    *output_file << "file I/O error! at " << text_buffer;
    passed = false;
  }
  *output_file << "\n";
  if (!passed) return;
  
  
  // extraction transcendent number
  *output_file << "  - extraction irrational number\n| LB ||  digits  || digits/sec ||     time    || memory(byte) |\n|----||----------||------------||-------------||--------------|\n";

  struct algo {
    const char *lb;
    const char *file;
    BigInteger *data;
    char (*extract)(BigInteger *);
  };
  std::vector<algo> algos {
    {"√2", "√2Digits.txt", new BigInteger[5]{1,2,4, 3, 1}, [](BigInteger *s) -> char {
      // a, b, c , d, e
      while (s[1] * s[2] < s[4] * s[3] * 10000) {
        s[4] *= s[3];
        s[0] *= s[2];
        s[0] += s[4];
        s[1] *= s[2];
  
        s[2] += 4;
        s[3] += 2;
      }
      char r = (char)int (s[0].div_mod (s[1]));
      s[0] *= 5;
      s[4] *= 5;
      s[1] >>= 1;
      return r;
    }},// √2
    {" e", "eDigits.txt", new BigInteger[4]{1,1,1,1}, [](BigInteger *s) -> char {
      // a, b, c, d
      while ((s[1] * s[2]) < (s[3] * 10000)) {
        s[0] *= s[2];
        s[0] += s[3];
        s[1] *= s[2];
        ++s[2];
      }
      char r = (char)int (s[0].div_mod (s[1]));
      s[0] *= 10;
      s[3] *= 10;
      return r;
    }},// e
    {" π", "piDigits.txt", new BigInteger[6]{1,6,3,2,5,3}, [](BigInteger *s) -> char {
      // q, r, t, k, l, n
     // do math
      while (s[0] * 4 + s[1] - s[2] >= s[2] * s[5]) {
        s[2] *= s[4];
        s[5] = s[3];
        s[5] *= 7;
        s[5] += 2;
        s[5] *= s[0];
        s[5] += s[1] * s[4];
        s[5] /= s[2];
        s[1] += s[0] * 2;
        s[1] *= s[4];
        s[0] *= s[3];
        ++s[3];
        s[4] += 2;
      }
      char r = static_cast<char> ((int)s[5]);
      s[0] *= 10;
      s[1] -= s[2] * s[5];
      s[1] *= 10;
      s[5] = s[0] * 3;
      s[5] += s[1];
      s[5] /= s[2];
      return r;
    }},// pi
  };
  // pi proof
  size_t digit_index, digit_readed;
  char result;
  unsigned long generated;
  // time proof
  simple_time_t counted_time;
  while (!algos.empty()) {
    algo A = algos.back();
    generated = 0;
    digit_index = 0;
    digit_readed = 0;
    sprintf (text_buffer, "%s/%s", data_address, A.file);
    FILE *file_digits = fopen (text_buffer, "r");
    if (file_digits) {
      try {
        ct.start ();
        do {
          if (digit_index >= digit_readed) {
            digit_index = 0;
            digit_readed = fread (text_buffer, 1, 2048, file_digits);
          }
          if (!digit_readed) {
            if (feof (file_digits)) throw "end of file";
            if (ferror (file_digits)) throw strerror (errno);
            throw "cannot get digits from file";
          }
          result = A.extract (A.data) + '0';
          if (result != text_buffer[digit_index++]) throw "wrong result";
          ++generated;
          // print result profiling
          *output_file << std::flush << "\r| " << A.lb << " || ";
          *output_file << std::setfill ('0') << std::setw ( 8) << generated << " || ";
          *output_file << std::setfill ('0') << std::setw (10) << std::fixed << std::setprecision (3) << ((double)generated / (double)counted_time.to_sec ()) << " || ";
          *output_file << std::setfill (' ') << std::setw (12) << std::right << (counted_time = ct.end()) << "|| ";
          *output_file << std::setfill ('0') << std::setw (12) << 99999;
        } while (
#ifdef TIME
          counted_time.to_sec () < TIME
#else
          true
#endif
          );
      } catch (const char *e) {
        *output_file << std::flush << "\r  Err: " << std::setfill (' ') << std::setw (20) << e << " digits: " << generated;
      } catch (...) {
        *output_file << std::flush << "\r  Err: " << std::setfill (' ') << std::setw (20) << "unknown in digits: " << generated;
      }
      fclose (file_digits);
    } else {
      *output_file << "  Err: file I/O error";
    }
    *output_file << " |\n";
    delete[] A.data;
    algos.pop_back();
  }
}
