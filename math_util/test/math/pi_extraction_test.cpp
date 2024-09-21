#include "BigInteger.hpp"
//#include "clock_adjustment.hpp"

#include <cstring>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
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
  virtual size_t size () {
    return sizeof (base_ex);
  }
  ~base_ex () {}
};

struct extract_algo : public base_ex {
private:
  BigInteger q = 1,
             r = 6,
             t = 3,
             k = 2,
             l = 5,
             n = 3;

public:
  extract_algo () {}
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
  size_t size () {
    return sizeof (q) + sizeof (r) + sizeof (t) + sizeof (k) + sizeof (l) + sizeof (n);
  }
  ~extract_algo () {}
};

struct spigot_algo : public base_ex {
private:
  unsigned int *A;
  bool growth = false;
  unsigned int len, predigit = 3, nines = 0;
  // safe digit for temp result, predigit, nines, result, check
  // temp
  unsigned tempResult, x, i;
  char result;

public:
  spigot_algo (const unsigned int _len) {
    len = _len + 10;
    A = new unsigned int[len]{0, 2, 2, 4, 3, 10, 1, 13, 12, 1};
    std::fill (A + 10, A + len, 20);
  }
  char extract () override {
    if (nines) {
      --nines;
      return growth ? 0 : 9;
    }
    for (;;) {
      tempResult = 0;
      x = 0, i = len;
      while (--i) {
        x += 10 * A[i];
        tempResult = x / (2 * i + 1);
        A[i] = x % (2 * i + 1);
        x = tempResult * i;
      }
      x += 10 * A[0];
      A[0] = tempResult % 10;
      tempResult = x / 10;

      if (tempResult != 9)
        break;

      ++nines;
    }
    growth = (tempResult == 10);
    result = predigit + growth;
    predigit = !growth * tempResult;
    return result;
  }
  size_t size () override {
    return sizeof (this) + sizeof (unsigned int) * len;
  }
  ~spigot_algo () {
    delete[] A;
  }
};

#define TIME 1.0  
#define BUFFER_BYTE_SIZE 4096

bool pi_extraction_test (char *d) {
  std::cout << "Start π Extraction Test Generator" << std::endl;
  bool passed = true;
  base_ex *algos[]{
      new extract_algo (),
      new spigot_algo (250000)};
  // Draw table header
  std::cout << "|   π(digits)   || rate(digits/sec) ||   memory(byte)   |\n";
  std::cout << "|---------------||------------------||------------------|\n";

  // pi proof
  char piBuffer[BUFFER_BYTE_SIZE];
  size_t piIndex, piReaded;
  char result;
  unsigned long long generated = 0;
  // time proof
  long double elapsed_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_timed, now_timed;
  for (base_ex *algo : algos) {
    std::cout << "| ";
    try {
      FILE *fpi = fopen ((std::string (d) + std::string ("/piDigits.txt")).c_str (), "r");
      if (!fpi) throw std::logic_error ("file not found");
      piIndex = 0;
      piReaded = 0;
      start_timed = std::chrono::high_resolution_clock::now ();
      do {
        if (piIndex >= piReaded) {
          piIndex = 0;
          piReaded = fread (piBuffer, 1, BUFFER_BYTE_SIZE, fpi);
        }
        if (piReaded == 0) {
          if (feof (fpi))
            throw std::logic_error ("end of file");
          if (ferror (fpi))
            throw std::logic_error (strerror (errno));
          throw std::logic_error ("cannot get digits from file");
        }
        result = algo->extract () + '0';
        if (result != piBuffer[piIndex++]) {
          throw std::logic_error ("wrong pi result");
        }
        ++generated;
        now_timed = std::chrono::high_resolution_clock::now ();
        elapsed_time = std::chrono::duration<double> (now_timed - start_timed).count () ;
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
  std::cout << "Ended π Test Generator" << std::endl;
  return passed;
}
