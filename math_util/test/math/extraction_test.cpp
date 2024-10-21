#include "BigInteger.hpp"
//#include "clock_adjustment.hpp"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

// algorithm list
struct base_ex {
  base_ex () {}
  virtual char extract () {
    return -1;
  }
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
    while ((q * 4 + r - t) >= (t * n)) {
      t *= l;
      n = q;
      n *= k;
      n *= 7;
      n += q * 2;
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
  const char *testFile () override { return "piDigits.txt"; }
  size_t size () {
    return sizeof (q) + sizeof (r) + sizeof (t) + sizeof (k) + sizeof (l) + sizeof (n);
  }
  ~pi_algo () {}
};
/*
          1     1     1     1               1
e => 1 + --- + --- + --- + ---- + ...... + ----
          1!    2!   3!     4!              n!

          1 > (a*c + d) * 100 /(b*c)
*/
struct e_algo : public base_ex {
private:
  BigInteger a = 2, // nominator
      b = 1,        // denominator
      c = 2,        // counter
      d = 1,        // base digit
      e = 0;        // result hold

public:
  e_algo () {}
  char extract () override {
    while (((a * c + d) * 100) < (b * c)) {
      a *= c;
      a += d;
      b *= c;
      c++;
    }
    e = a / b;
    char result = static_cast<char> ((int)e);
    a %= b;
    d *= 10;
    a *= 10;
    return result;
  }
  const char *testFile () override { return "eDigits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d) + sizeof (e); }
  ~e_algo () {}
};

#define TIME 10.0
#define BUFFER_BYTE_SIZE 4096

bool extraction_test (const char *d) {
  std::cout << "Start Extraction Test Generator" << std::endl;
  bool passed = true;
  base_ex *algos[]{
      new pi_algo (),
      new e_algo ()};
  // Draw table header
  std::cout << "|     digits    || rate(digits/sec) ||   memory(byte)   |\n";
  std::cout << "|---------------||------------------||------------------|\n";

  // pi proof
  char buff[BUFFER_BYTE_SIZE];
  size_t piIndex, piReaded;
  char result;
  unsigned long long generated = 0;
  // time proof
  long double elapsed_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_timed, now_timed;
  for (base_ex *algo : algos) {
    std::cout << "| ";
    try {
      sprintf (buff, "%s/%s", d, algo->testFile ());
      FILE *fpi = fopen (buff, "r");
      if (!fpi) throw std::logic_error ("file not found");
      piIndex = 0;
      piReaded = 0;
      start_timed = std::chrono::high_resolution_clock::now ();
      do {
        if (piIndex >= piReaded) {
          piIndex = 0;
          piReaded = fread (buff, 1, BUFFER_BYTE_SIZE, fpi);
        }
        if (piReaded == 0) {
          if (feof (fpi))
            throw std::logic_error ("end of file");
          if (ferror (fpi))
            throw std::logic_error (strerror (errno));
          throw std::logic_error ("cannot get digits from file");
        }
        result = algo->extract () + '0';
        if (result != buff[piIndex++]) {
          throw std::logic_error ("wrong pi result");
        }
        ++generated;
        now_timed = std::chrono::high_resolution_clock::now ();
        elapsed_time = std::chrono::duration<double> (now_timed - start_timed).count ();
      } while (elapsed_time < TIME);
      {
        // print result profiling
        std::cout << std::setfill ('0') << std::setw (13) << generated << " || ";
        std::cout << std::setfill ('0') << std::setw (16) << std::fixed << std::setprecision (5) << ((long double)generated / elapsed_time) << " || ";
        std::cout << std::setfill ('0') << std::setw (16) << algo->size ();
      }
      fclose (fpi);
    } catch (const std::exception &e) {
      std::cout << std::setfill (' ') << std::setw (43) << std::internal << "Error: " << e.what ();
      passed &= false;
    }
    std::cout << " |" << std::endl;
    delete algo;
  }
  std::cout << "Ended Test Generator" << std::endl;
  return passed;
}
