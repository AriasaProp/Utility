#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main() {
  if (0) {
    long double prime[8] = {2,3,5,7,11,13,17,19};
    uint32_t H0_ref[8] = {
      0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
  
    for (size_t i = 0; i < 8; i++) {
      long double x = sqrtl(prime[i]);
      x = x - floorl(x);          // frac(sqrt(p))
      x = x * 4294967296.0L;     // * 2^32
      uint32_t ti = (uint32_t)floorl(x); // floor
  
      printf("i=%zu  computed=%08x  ref=%08x  %s\n",
             i, ti, H0_ref[i], (ti==H0_ref[i]) ? "OK" : "NO");
    }
  }
  if (1) {
    long double prime[64] = {
        2,  3,  5,  7, 11, 13, 17, 19,
       23, 29, 31, 37, 41, 43, 47, 53,
       
       59, 61, 67, 71, 73, 79, 83, 89,
       97,101,103,107,109,113,127,131,
      137,139,149,151,157,163,167,173,
      179,181,191,193,197,199,211,223,
      227,229,233,239,241,251,257,263,
      269,271,277,281,283,293,307,311
    };
    static const uint32_t K[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  
  
    for (size_t i = 0; i < 64; i++) {
      long double x = cbrt(prime[i]);
      x -= floorl(x);          // frac(sqrt(p))
      x *= 4294967296.0L;     // * 2^32
      uint32_t ti = (uint32_t)floorl(x); // floor
  
      printf("i=%zu  computed=%08x  ref=%08x  %s\n",
             i, ti, K[i], (ti==K[i]) ? "OK" : "NO");
    }
  }
  return 0;
}
