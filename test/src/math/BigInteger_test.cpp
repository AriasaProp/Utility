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
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

// undef TIME to reach limit of file digits
#define TIME 2
// undef UPDATE_RATE to update data each digits found
#define UPDATE_RATE 0.6
enum stop_type : unsigned char {
  NO_END = 0,
#ifdef TIME
  TIMEOUT = 1,
#endif
  END_OF_FILE = 2,
};

extern std::ostream *output_file;
extern const unsigned int TEXT_BUFFER_SIZE;
extern char *text_buffer;
extern const char *data_address;

void BigInteger_test () {
  simple_timer_t ct;
  bool passed = true;
  unsigned long cnt = 0;
  
  *output_file << "BigInteger Test:\n";
  {
    // basic operation test
    *output_file << "  - basic: ";
    
    sprintf(text_buffer, "%s/BigIntegerTest.txt", data_address);
    FILE *file = fopen(text_buffer, "r");
    if (!file) {
      *output_file << "file I/O error! at " << text_buffer;
      passed = false;
      goto basic_end;
    }
    try {
  		ct.start();
  		int ch = 0;
      while((ch = fgetc(file)) != EOF) {
        cnt++;
  #define EXTRACT(N) if (!fscanf(file, " %s", text_buffer)) throw "format test wrong!"; \
  BigInteger N(text_buffer)
  #define COMMON3(D) if (!fscanf(file, " #%[^\n]", text_buffer)) throw "format test wrong!"; \
  fgetc(file); if (A D B != C) throw text_buffer; \
  break
      	switch (ch) {
      		case 'A': {
      		  EXTRACT(A); EXTRACT(B); EXTRACT(C);
      		  if (!fscanf(file, " #%[^\n]", text_buffer)) throw "format test wrong!";
      		  fgetc(file);
      		  if (A + B != C) throw text_buffer;
      			break;
      		}
      		case 'B': {
      		  EXTRACT(A); EXTRACT(B); EXTRACT(C);
      		  if (!fscanf(file, " #%[^\n]", text_buffer)) throw "format test wrong!";
      		  fgetc(file);
      		  if (A - B != C) throw text_buffer;
      			break;
      		}
      		case 'C': {
      		  EXTRACT(A); EXTRACT(B); EXTRACT(C);
      		  if (!fscanf(file, " #%[^\n]", text_buffer)) throw "format test wrong!";
      		  fgetc(file);
      		  if (A * B != C) throw text_buffer;
      			break;
      		}
      		case 'D': {
      		  EXTRACT(A); EXTRACT(B); EXTRACT(C);
      		  if (!fscanf(file, " #%[^\n]", text_buffer)) throw "format test wrong!";
      		  fgetc(file);
      		  if (A / B != C) throw text_buffer;
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
      		  EXTRACT(A); EXTRACT(B); EXTRACT(C);
      		  if (!fscanf(file, " #%[^\n]", text_buffer)) throw "format test wrong!";
      		  fgetc(file);
      		  if (A % B != C) throw text_buffer;
      			break;
      		}
      		case 'H': {
      		  EXTRACT(A);
      	    EXTRACT(B);
      	    EXTRACT(C);
      	    EXTRACT(D);
      		  BigInteger A1 = A;
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
basic_end:
    *output_file << "\n";
    if (!passed) return;
  }
  {
    // extraction transcendent number
    *output_file << "  - extraction irrational number\nLB | digits |   rate   |    time    |\n---|--------|----------|------------|\n";
    static const size_t BIG_TENS = sizeof(word) * 8;
  
    struct algo {
      const char *lb;
      const char *file;
      BigInteger *data;
      char (*extract)(BigInteger *);
    };
    std::vector<algo> algos {
      {"√2", "√2Digits.txt", new BigInteger[5]{1,2,4, 3, 1}, [](BigInteger *s) -> char {
        // a, b, c , d, e
        while (s[1] * s[2] < ((s[4] * s[3]) << BIG_TENS)) {
          s[4] *= s[3];
          s[0] *= s[2];
          s[0] += s[4];
          s[1] *= s[2];
    
          s[2] += 4;
          s[3] += 2;
        }
        char r = (char) s[0].div_mod (s[1]);
        s[0] *= 5;
        s[4] *= 5;
        s[1] >>= 1;
        return r;
      }},// √2
      {" e", "eDigits.txt", new BigInteger[4]{9864101,3628800,11,1}, [](BigInteger *s) -> char {
        // a, b, c, d
        /*
         1      9864101      1
        ----  = ------- + -------
         n!        10!        10!
        
        */
        while ((s[1] * s[2]) < (s[3] << BIG_TENS)) {
          s[0] *= s[2];
          s[0] += s[3];
          s[1] *= s[2];
          ++s[2];
        }
        char r = (char)s[0].div_mod (s[1]);
        s[0] *= 10;
        s[3] *= 10;
        return r;
      }},// e
      {" π", "piDigits.txt", new BigInteger[8]{1,6,3,2,5,3}, [](BigInteger *s) -> char {
        // q, r, t, k, l, n, (m, o)
        // do compare
        while ((s[0] << (size_t)2) + s[1] > (s[5] + 1) * s[2]) {
          // do math
          s[2] *= s[4];
          s[5] = s[3] * 7;
          s[5] += 2;
          s[5] *= s[0];
          s[5] += s[1] * s[4];
          s[5] /= s[2];
          s[1] += s[0] << (size_t)1;
          s[1] *= s[4];
          s[0] *= s[3];
          ++s[3];
          s[4] += 2;
        }
        char r = (char) s[5];
        s[0] *= 10;
        s[1] -= s[2] * s[5];
        s[1] *= 10;
        s[5] = s[0] * 3;
        s[5] += s[1];
        s[5] /= s[2];
        return r;
      }},// pi
    };
    stop_type end_type;
    char result;
    void *file_digits;
    const char *file_digits_current, *file_digits_end;
    struct stat bstat;
    int fd;
    // time proof
    simple_time_t counted_time;
#ifdef UPDATE_RATE
    simple_timer_t pt;
#endif
    do {
      end_type = NO_END;
      algo A = algos.back();
      cnt = 0;
      sprintf (text_buffer, "./%s/%s", data_address, A.file);
      if ((fd = open(text_buffer, O_RDONLY | S_IRUSR)) < 0) {
        strcpy(text_buffer, "file I/O error");
        goto end_ex;
      }
      *text_buffer = 0;
      if (fstat(fd, &bstat) == -1) {
        close(fd);
        strcpy(text_buffer, "file stat error");
        goto end_ex;
      }
      if (!(file_digits = mmap(NULL, bstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0))) {
        close(fd);
        strcpy(text_buffer, "mapping stat error");
        goto end_ex;
      }
      file_digits_current = (const char*)file_digits;
      file_digits_end = file_digits_current + bstat.st_size;
      close(fd);
      ct.start ();
  #ifdef UPDATE_RATE
      pt.start ();
  #endif
  print_ex:
      // print result profiling
      *output_file << std::flush << "\r" << A.lb << " | ";
      *output_file << std::setfill ('0') << std::setw ( 6) << cnt << " | ";
      counted_time = ct.end();
      *output_file << std::setfill ('0') << std::setw ( 8) << std::fixed << std::setprecision (1) << ((double)cnt / counted_time.to_sec ()) << " | ";
      *output_file << std::setfill (' ') << std::setw (10) << std::right << counted_time;
      switch (end_type) {
        default:
        case NO_END:
          break;
        case END_OF_FILE:
          *output_file << " | end of file";
          goto end_exf;
  #ifdef TIME
        case TIMEOUT:
          *output_file << " | timeout";
          goto end_exf;
  #endif
      }
  start_ex:
      try {
        result = A.extract (A.data);
      } catch (const char *e) {
        strcpy(text_buffer, e);
        goto end_exf;
      }
      if ((*file_digits_current - result) != '0') {
        sprintf(text_buffer, "wrong, result %d, correct %c", (int)result, *file_digits_current);
        goto end_exf;
      }
      ++cnt;
      if (++file_digits_current >= file_digits_end) {
        end_type = END_OF_FILE;
        goto print_ex;
      }
  #ifdef TIME
      // end when time passed 
      if ((ct.end()).to_sec () > TIME) {
        end_type = TIMEOUT;
        goto print_ex;
      }
  #endif
  #ifdef UPDATE_RATE
      // do print state when update rate passed 
      if (pt.end().to_sec () < UPDATE_RATE) goto start_ex;
      pt.start ();
  #endif
      goto print_ex;
  end_exf:
      munmap(file_digits, bstat.st_size);
  end_ex:
      if (*text_buffer) 
        *output_file << std::flush << "\re: " << std::setfill (' ') << std::setw (20) << text_buffer << " at: " << cnt;
      *output_file << " |\n";
      delete[] A.data;
      algos.pop_back();
    } while (algos.size());
  }
}
