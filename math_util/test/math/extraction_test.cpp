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
/*
    2   2*1   2*1*2   2*1*2*3   2*1*2*3*4
π = - + --- + ----- + ------- + --------- + .....
    1   1*3   1*3*5   1*3*5*7   1*3*5*7*9

          2*x!
π = £   -------
   x=0  (2x+1)!!
   
   c      1
   bd  > 100000
*/

struct pi_algo : public base_ex {
private:
  BigInteger a = 2, b = 1, c = 2, d = 3, i = 1;

public:
  pi_algo () {}
  char extract () override {
    // do math
    while (c * 10000000 < b * d) {
      a *= d;
      b *= d;
      a += c;

      c *= ++i;
      d += 2;
    }
    char result = static_cast<char> (a.div_mod (b));
    a *= 10;
    c *= 10;
    return result;
  }
  const char *lbl () override { return "π"; }
  const char *testFile () override { return "piDigits.txt"; }
  size_t size () {
    return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d) + sizeof (i);
  }
  ~pi_algo () {}
};
/*
            1
e = £     ------
   x=0      x!

    1    1    1
e = -- + -- + -- + ....
    0!   1!   2!

    1     1     1
e = -- + --- + ---- + .....
    1     1    1*2

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
    while (d * 10000 > (b * c)) {
      a *= c;
      a += d;
      b *= c;
      ++c;
    }
    char result = static_cast<char> (a.div_mod (b));
    b /= 10;
    return result;
  }
  const char *lbl () override { return "e"; }
  const char *testFile () override { return "eDigits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d); }
  ~e_algo () {}
};
/*
                  (2x+1)!!
√2 = £     -------------------
   x=0      (x!) * 2^(2x+1)

      1        1*3         1*3*5
√2 = ----- + -------- + ----------- + ....
     1 * 2   1 * 2^3    (1*2) * 2^5

*/
struct root2_algo : public base_ex {
private:
  BigInteger a = 1, // nominator
      b = 2,        // denominator

      c = 1, // counter
      d = 3, // odd counter
      e = 3; // factorial odd

public:
  root2_algo () {}
  char extract () override {
    while (b * c < e * 250000) {
      a *= c * 4;
      a += e;
      b *= c * 4;

      e *= d;
      ++c;
      d += 2;
    }
    char result = static_cast<char> (a.div_mod (b));
    a *= 5;
    e *= 5;
    b >>= 1;
    return result;
  }
  const char *lbl () override { return "√2"; }
  const char *testFile () override { return "√2Digits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d) + sizeof (e); }
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
