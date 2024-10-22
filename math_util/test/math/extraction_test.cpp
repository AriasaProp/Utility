#include "BigInteger.hpp"
//#include "clock_adjustment.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>

// Set TIME to 0.0 to reach limit of file digits
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
  const char *lbl () override { return "π"; }
  const char *testFile () override { return "piDigits.txt"; }
  size_t size () {
    return sizeof (q) + sizeof (r) + sizeof (t) + sizeof (k) + sizeof (l) + sizeof (n);
  }
  ~pi_algo () {}
};
/*
     1     1     1x2     1x2x3           n!
π = --- + --- + ----- + ------- + ... + -----
     1    1x3   1x3x5   1x3x5x7          odd!
*/
struct pis_algo : public base_ex {
private:
  BigInteger a = 1, // nominator
      b = 1,        // denominator

      c = 1, // counter
      d = 3, // counter odd

      e = 1, // factorial

      g = 1; // base digit

public:
  pis_algo () {}
  char extract () override {
    while ((d * b) < (e * c * g * 1000)) {
      e *= c;
      a += e;
      b *= d;

      d += 2;
      ++c;
    }
    char result = char (a / b);
    g *= 10;
    a *= 10;
    return result;
  }
  const char *lbl () override { return "π"; }
  const char *testFile () override { return "piDigits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d); }
  ~pis_algo () {}
};
struct e_algo : public base_ex {
private:
  BigInteger a = 2, // nominator
      b = 1,        // denominator
      c = 2,        // counter
      d = 1;        // base digit

public:
  e_algo () {}
  char extract () override {
    while ((d * d * 1000) > (b * c)) {
      a *= c;
      a += d;
      b *= c;
      ++c;
    }
    char result = char (a / b)
        a %= b;
    d *= 10;
    a *= 10;
    return result;
  }
  const char *lbl () override { return "e"; }
  const char *testFile () override { return "eDigits.txt"; }
  size_t size () { return sizeof (a) + sizeof (b) + sizeof (c) + sizeof (d); }
  ~e_algo () {}
};

bool extraction_test (const char *d) {
  std::cout << "Start Extraction Test Generator" << std::endl;
  bool passed = true;
  base_ex *algos[]{
      new pi_algo (),
      new pis_algo (),
      new e_algo ()};
  // Draw table header
  std::cout << " |    digits    || rate(digits/sec) ||   memory(byte)   |\n";
  std::cout << "-|--------------||------------------||------------------|\n";

  // pi proof
  char buff[BUFFER_BYTE_SIZE];
  size_t piIndex, piReaded;
  char result;
  unsigned long long generated = 0;
  // time proof
  long double elapsed_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_timed, now_timed;
  for (base_ex *algo : algos) {
    std::cout << algo->lbl () << "| ";
    try {
      sprintf (buff, "%s/%s", d, algo->testFile ());
      FILE *fpi = fopen (buff, "r");
      if (!fpi) throw "file not found";
      piIndex = 0;
      piReaded = 0;
      start_timed = std::chrono::high_resolution_clock::now ();
      do {
        if (piIndex >= piReaded) {
          piIndex = 0;
          piReaded = fread (buff, 1, BUFFER_BYTE_SIZE, fpi);
        }
        if (!piReaded) {
          if (feof (fpi)) throw "end of file";
          if (ferror (fpi)) throw strerror (errno);
          throw "cannot get digits from file";
        }
        result = algo->extract () + '0';
        if (result != buff[piIndex++]) throw "wrong result";
        ++generated;
#if TIME == 0.0
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
      fclose (fpi);
    } catch (const char *e) {
      std::cout << std::setfill (' ') << std::setw (43) << std::internal << "Error: " << e;
      passed &= false;
    }
    std::cout << " |" << std::endl;
    delete algo;
  }
  std::cout << "Ended Test Generator" << std::endl;
  return passed;
}
