#include "math/BigInteger.hpp"
#include "simple_profiling.hpp"

#include <cmath>
#include <cstdlib>
#include <ostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>

// undef TIME to reach limit of file digits
#define TIME 1.0

extern std::ostream *output_file;
extern char text_buffer[2048];
extern const char *data_address;

static bool basic_test(){
  *output_file << "basic -> ";
  bool passed = true;
  simple_timer_t ct;
  
  // do calculation
  sprintf(text_buffer, "%s/BigIntegerTest.txt", data_address);
  FILE *file = fopen(text_buffer, "r");
  if (file) {
		size_t cnt= 0;
    try {
  		ct.start();
  		int ch = 0;
      while((ch = fgetc(file)) != EOF) {
        cnt++;
      	switch (ch) {
      		case 'A': {
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A+B != C) {
  			  	  *output_file << "result: " << (A+B) << " should be " << C;
  			  		throw text_buffer;
  			  	}
      			break;
      		}
      		case 'B': {
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A-B != C) {
  			  	  *output_file << "result: " << (A-B) << " should be " << C;
  			  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'C': {
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if ((A*B) != C) {
  			  	  *output_file << "result: " << (A*B) << " should be " << C;
    		  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'D': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A/B != C) {
  			  	  *output_file << "result: " << (A/B) << " should be " << C;
  			  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'E': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      	    int B;
      	    if (!fscanf(file, " %d", &B))
      	      throw "format test wrong!";
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A.pow(B) != C) {
  			  	  *output_file << "result: " << (A^B) << " should be " << C;
  			  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'F': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A.sqrt() != C)
  			  		throw text_buffer;
      			break;
      		}
      		case 'G': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer);
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A%B != C) {
  			  	  *output_file << "result: " << (A%B) << " should be " << C;
  			  		throw text_buffer;
  			  	}
      			break;
      		}
      		case 'H': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger A(text_buffer), A1 = A;
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger C(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    BigInteger D(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    fgetc(file);
  			  	if (A1.div_mod(B) != C || (A1 != D)) {
  			  	  *output_file << "result: " << (A.div_mod(B)) << " should be " << C << " and " << A << " should be " << D;
  			  		throw text_buffer;
  			  	}
      			break;
      		}
      		default:
      			throw "unexpected value";
      	}
      }
      *output_file << ct.end();
    } catch (const std::exception *msg) {
      *output_file << "Error " << cnt << " -> " << msg->what();
      passed = false;
    } catch (const char *msg) {
      *output_file << "Error " << cnt << " -> " << msg;
      passed = false;
    }
    fclose(file);
  } else {
    *output_file << "file I/O error! at " << text_buffer;
    passed = false;
  }
  *output_file << "\n";
  return passed;
}

// algorithm list
struct base_ex {
  base_ex () {}
  virtual char extract () = 0;
  virtual const char *lbl () = 0;
  virtual const char *testFile () = 0;
  virtual size_t size () = 0;
  virtual ~base_ex () {}
};

struct pi_algo : public base_ex {
private:
  BigInteger q = 1, r = 6, t = 3, k = 2, l = 5, n = 3;

public:
  pi_algo () {}
  char extract () override {
    // do math
    while (q * 4 + r - t >= t * n) {
      t *= l;
      n = k;
      n *= 7;
      n += 2;
      n *= q;
      n += r * l;
      n /= t;
      r += q * 2;
      r *= l;
      q *= k;
      ++k;
      l += 2;
    }
    char result = static_cast<char> ((int)n);
    q *= 10;
    r -= t * n;
    r *= 10;
    n = q;
    n *= 3;
    n += r;
    n /= t;
    return result;
  }
  const char *lbl () override { return " π"; }
  const char *testFile () override { return "piDigits.txt"; }
  size_t size () override {
    return sizeof (q) + sizeof (r) + sizeof (t) + sizeof (k) + sizeof (l) + sizeof (n);
  }
  ~pi_algo () override {}
};
/*
           1
e = £     ------
   x=0      x!
*/
struct e_algo : public base_ex {
private:
  BigInteger a = 1, // nominator
      b = 1,        // denominator
      c = 1,        // counter
      d = 1;        // base digit

public:
  e_algo () {}
  char extract () override {
    while (d >= (b * c)) {
      a *= c;
      a += d;
      b *= c;
      ++c;
    }
    char result = (char)int (a.div_mod (b));
    a *= 10;
    d *= 10;
    return result;
  }
  const char *lbl () override { return " e"; }
  const char *testFile () override { return "eDigits.txt"; }
  size_t size () override { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d); }
  ~e_algo () override {}
};
/*
          (2x+1)!!
√2 = £     --------
   x=0     x!2^(2x+1)
*/
struct root2_algo : public base_ex {
private:
  BigInteger a = 1, // nominator
      b = 2,        // denominator

      c = 4, // counter 4x
      d = 3, // counter odd
      e = 1; // factorial

public:
  root2_algo () {}
  char extract () override {
    while ((b * c) < (e * d)) {
      e *= d;
      a *= c;
      a += e;
      b *= c;

      c += 4;
      d += 2;
    }
    char result = (char)int (a.div_mod (b));
    a *= 5;
    e *= 5;
    b /= 2;
    return result;
  }
  const char *lbl () override { return "√2"; }
  const char *testFile () override { return "√2Digits.txt"; }
  size_t size () override { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d) + sizeof (e); }
  ~root2_algo () override {}
};

static bool extraction_test () {
  // Draw table header
  *output_file << "exercise\n| LB ||  digits  ||rate(digits/sec)||    time    || memory(byte) |\n|----||----------||----------------||------------||--------------|\n";

  bool passed = true;
  base_ex *algos[]{
	  new pi_algo (),
	  new e_algo (),
	  new root2_algo ()
  };
  // pi proof
  size_t digit_index, digit_readed;
  char result;
  unsigned long generated;
  // time proof
  simple_timer_t counter_time;
  simple_time_t counted_time;
  for (base_ex *algo : algos) {
    generated = 0;
    digit_index = 0;
    digit_readed = 0;
    sprintf (text_buffer, "%s/%s", data_address, algo->testFile ());
    FILE *file_digits = fopen (text_buffer, "r");
    if (file_digits) {
      counter_time.start ();
      try {
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
          result = algo->extract () + '0';
          if (result != text_buffer[digit_index++]) throw "wrong result";
          ++generated;
          // print result profiling
          *output_file << std::flush << "\r| " << algo->lbl () << " || ";
          *output_file << std::setfill ('0') << std::setw ( 8) << generated << " || ";
          *output_file << std::setfill ('0') << std::setw (14) << std::fixed << std::setprecision (4) << (1.0 * generated / counted_time.to_sec ()) << " || ";
          *output_file << std::setfill (' ') << std::setw (11) << std::right << (counted_time = counter_time.end()) << "|| ";
          *output_file << std::setfill ('0') << std::setw (12) << algo->size ();
        } while (
  #ifdef TIME
          counted_time.to_sec () < TIME
  #else
          true
  #endif
          );
      } catch (const std::exception &e) {
        *output_file << std::flush << "\r  Err: " << std::setfill (' ') << std::setw (20) << e.what() << " digits: " << generated;
        passed = false;
      } catch (const char *e) {
        *output_file << std::flush << "\r  Err: " << std::setfill (' ') << std::setw (20) << e << " digits: " << generated;
        passed = false;
      }
      fclose (file_digits);
    } else {
      *output_file << "  Err file I/O error";
    }
    *output_file << " |\n";
    delete algo;
  }
  return passed;
}

bool BigInteger_test () {
  *output_file << "BigInteger Test:\n";
  bool r = basic_test() && extraction_test();
  *output_file << "\n" << mem_a << std::endl;
  return r;
}