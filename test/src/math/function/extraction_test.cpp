#include "math/BigInteger.hpp"
#include "util/simple_clock.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <iomanip>
#include <memory>

// undef TIME to reach limit of file digits
#define TIME 10.0

extern std::ostream *output_file;
extern char text_buffer[2048];
extern const char *data_address;

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
  BigInteger q = 1,
             r = 6,
             t = 3,
             k = 2,
             l = 5,
             n = 3;

public:
  pi_algo () {}
  char extract () override {
    // do math
    while (q * 4 + r - t >= t * n) {
      t *= l;
      n = q * (k * 7 + 2);
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
    n = q * 3;
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
    while ((d * 10000) > (b * c)) {
      a *= c;
      a += d;
      b *= c;
      ++c;
    }
    char result = (char)int (a.div_mod (b));
    d *= 10;
    a *= 10;
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
    while ((b * c) < (e * d * 10000)) {
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
    b >>= 1;
    return result;
  }
  const char *lbl () override { return "√2"; }
  const char *testFile () override { return "√2Digits.txt"; }
  size_t size () override { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d) + sizeof (e); }
  ~root2_algo () override {}
};

bool extraction_test () {
  // Draw table header
  *output_file << "Start Extraction Test\n| LB ||  digits  ||rate(digits/sec)||    time    || memory(byte) |\n|----||----------||----------------||------------||--------------|\n";

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
    *output_file << "| " << algo->lbl () << " || ";
    generated = 0;
    digit_index = 0;
    digit_readed = 0;
    try {
      sprintf (text_buffer, "%s/%s", data_address, algo->testFile ());
      FILE *file_digits = fopen (text_buffer, "r");
      if (!file_digits)
        throw "file not found";
      counter_time.start ();
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
#ifdef TIME
      } while ((counted_time = counter_time.end ()).to_sec () < TIME);
#else
      } while (true);
      counted_time = counter_time.end ();
#endif
      // print result profiling
      *output_file << std::setfill ('0') << std::setw (8) << generated << " || ";
      *output_file << std::setfill ('0') << std::setw (14) << std::fixed << std::setprecision (4) << (1.0 * generated / counted_time.to_sec ()) << " || ";
      *output_file << std::setfill (' ') << std::setw (11) << std::right << counted_time << "|| ";
      *output_file << std::setfill ('0') << std::setw (12) << algo->size ();
      fclose (file_digits);
    } catch (const char *e) {
      *output_file << std::setfill (' ') << std::setw (66) << "Error: " << e << " digits: " << generated;
      passed &= false;
    }
    *output_file << " |" << std::endl;
    delete algo;
  }
  *output_file << std::endl;
  return passed;
}