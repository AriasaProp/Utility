#include "BigInteger.hpp"
//#include "clock_adjustment.hpp"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

// undef TIME to reach limit of file digits
#define TIME 10.0
#define BUFFER_BYTE_SIZE 4096

// algorithm list
struct base_ex {
  base_ex () {}
  virtual char extract () { return -1; }
  virtual const char *lbl () = 0;
  virtual const char *testFile () = 0;
  virtual size_t size () {
    return sizeof (base_ex);
  }
  ~base_ex () {}
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
  const char *lbl () override { return "π"; }
  const char *testFile () override { return "piDigits.txt"; }
  size_t size () {
    return sizeof (q) + sizeof (r) + sizeof (t) + sizeof (k) + sizeof (l) + sizeof (n);
  }
  ~pi_algo () {}
};
struct e_algo : public base_ex {
private:
  BigInteger a = 65, // nominator
      b = 24,        // denominator
      c = 5,         // counter
      d = 1;         // base digit

public:
  e_algo () {}
  char extract () override {
    while ((d * d * 1000) > (b * c)) {
      a *= c;
      a += d;
      b *= c;
      ++c;
    }
    char result = (char)int (a.div_mod (b));
    d *= 5;
    a *= 5;
    b >>= 1;
    return result;
  }
  const char *lbl () override { return "e"; }
  const char *testFile () override { return "eDigits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d); }
  ~e_algo () {}
};
struct root2_algo : public base_ex {
private:
  BigInteger a = 319, // nominator
      b = 256,        // denominator

      c = 4,   // counter
      d = 105; // factorial

public:
  root2_algo () {}
  char extract () override {
    while (b * c < d * (c * 2 + 1) * 25000) {
      d *= c * 2 + 1;
      a *= c * 4;
      a += d;
      b *= c * 4;
      ++c;
    }
    char result = (char)int (a.div_mod (b));
    a *= 5;
    d *= 5;
    b >>= 1;
    return result;
  }
  const char *lbl () override { return "√2"; }
  const char *testFile () override { return "√2Digits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d); }
  ~root2_algo () {}
};

bool extraction_test (const char *d) {
  std::cout << "Start Extraction Test Generator" << std::endl;
  bool passed = true;
  base_ex *algos[]{
      new pi_algo (),
      new e_algo (),
      new root2_algo ()};
  // Draw table header
  std::cout << "     |    digits    || rate(digits/sec) ||   memory(byte)   |\n";
  std::cout << "-----|--------------||------------------||------------------|\n";

  // pi proof
  char buff[BUFFER_BYTE_SIZE];
  size_t digit_index, digit_readed;
  char result;
  unsigned long long generated = 0;
  // time proof
  long double elapsed_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_timed, now_timed;
  for (base_ex *algo : algos) {
    std::cout << " " << std::setfill (' ') << std::setw (4) << algo->lbl () << " | ";
    try {
      sprintf (buff, "%s/%s", d, algo->testFile ());
      FILE *file_digits = fopen (buff, "r");
      if (!file_digits) [[unlikely]]
        throw "file not found";
      generated = 0;
      digit_index = 0;
      digit_readed = 0;
      start_timed = std::chrono::high_resolution_clock::now ();
      do {
        if (digit_index >= digit_readed) {
          digit_index = 0;
          digit_readed = fread (buff, 1, BUFFER_BYTE_SIZE, file_digits);
        }
        if (!digit_readed) [[unlikely]] {
          if (feof (file_digits)) throw "end of file";
          if (ferror (file_digits)) throw strerror (errno);
          throw "cannot get digits from file";
        }
        result = algo->extract () + '0';
        if (result != buff[digit_index++]) throw "wrong result";
        ++generated;
#ifndef TIME
      } while (true);
      now_timed = std::chrono::high_resolution_clock::now ();
      elapsed_time = std::chrono::duration<double> (now_timed - start_timed).count ();
#else
        now_timed = std::chrono::high_resolution_clock::now ();
        elapsed_time = std::chrono::duration<double> (now_timed - start_timed).count ();
      } while (elapsed_time < TIME);
#endif
      // print result profiling
      std::cout << std::setfill ('0') << std::setw (12) << generated << " || ";
      std::cout << std::setfill ('0') << std::setw (16) << std::fixed << std::setprecision (5) << ((long double)generated / elapsed_time) << " || ";
      std::cout << std::setfill ('0') << std::setw (16) << algo->size ();
      fclose (file_digits);
    } catch (const char *e) {
      std::cout << std::setfill (' ') << std::setw (43) << "Error: " << e << " digits: " << generated;
      passed &= false;
    }
    std::cout << " |" << std::endl;
    delete algo;
  }
  std::cout << "Ended Test Generator" << std::endl;
  return passed;
}
