#include "BigInteger.hpp"
//#include "clock_adjustment.hpp"

#include <chrono>
#include <cstdio>
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

#define TIME 10.0

bool pi_extraction_test () {
  std::cout << "Start π Extraction Test Generator" << std::endl;
  bool passed = true;
  base_ex *algos[]{
      new extract_algo (),
      new spigot_algo (250000)};
  // Draw table header
  std::cout << "|     π    ||    rate    ||   memory  |\n";
  std::cout << "|  digits  || digits/sec ||    byte   |\n";
  std::cout << "-------------------------------------------------------" << std::endl;
  for (base_ex *algo : algos) {
      try {
        unsigned long long generated = 0;
        double timed_average[3] = { 0.0000001};
        auto timed = std::chrono::high_resolution_clock::now ();
        auto whole_timed = std::chrono::high_resolution_clock::now ();
        do {
        	algo->extract ();
          ++generated;
        } while (wt < TIME);
      } catch (const std::exception &e) {
        std::cout << "\nError: " << e.what () << std::endl;
      	passed &= false;
      }
               {
            // print result profiling
            auto now = std::chrono::high_resolution_clock::now ();

            double t_av = (timed_average[0] = timed_average[1]);
            t_av += (timed_average[1] = timed_average[2]);
            t_av += (timed_average[2] = std::chrono::duration<double> (now - timed).count ());
            t_av = 3 / t_av;
            timed = now;
            double wt = std::chrono::duration<double> (now - whole_timed).count ();

            std::cout << std::flush << "\r| ";
            std::cout << std::setfill ('0') << std::setw (8) << generated << " || ";
            std::cout << std::setfill ('0') << std::setw (10) << std::fixed << std::setprecision (2) << t_av << " || ";
            std::cout << std::setfill ('0') << std::setw (9) << algo->size () << " |";

          }
    delete algo;
  }
  std::cout << "Ended π Test Generator" << std::endl;
  return passed;
}
