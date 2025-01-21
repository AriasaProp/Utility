#include "hash.hpp"
#include "simple_clock.hpp"

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ostream>

extern std::ostream *output_file;

// util
void print_bytes (const unsigned char *data, size_t dataLen, bool format = true) {
  for (size_t i = 0; i < dataLen; ++i) {
    *output_file << std::hex << std::setfill ('0') << std::setw (2) << (int)data[i];
    if (format) {
      *output_file << (((i + 1) % 16 == 0) ? "\n" : " ");
    }
  }
  *output_file << std::endl;
}
void print_bytes_reversed (const unsigned char *data, size_t dataLen, bool format = true) {
  for (size_t i = dataLen; i > 0; i--) {
    *output_file << std::hex << std::setw (2) << (int)data[i - 1];
    if (format) {
      *output_file << (((i - 1) % 16 == 0) ? "\n" : " ");
    }
  }
  *output_file << std::endl;
}
uint32_t Reverse32 (uint32_t value) {
  return (((value & 0x000000FF) << 24) |
          ((value & 0x0000FF00) << 8) |
          ((value & 0x00FF0000) >> 8) |
          ((value & 0xFF000000) >> 24));
}

void hexstr_to_intarray (const char *hexstr, uint32_t *outputloc) {
  for (size_t i = 0, j, k, l = strlen (hexstr); k < l; ++i) {
  	outputloc[i] = 0;
  	for (j = 0; j < 8 && k < l;++j, ++k) {
  		outputloc[i] <<= 4;
  		outputloc[i] |= hexstr[k] - 48 - (hexstr[k] > 9) * 39;
  	}
  }
}
// util end

// miner

// Converts bits to a 256-bit value that we can compare our hash against
void bits_to_difficulty (uint32_t bits, uint32_t *difficulty) {
  for (int i = 0; i < 8; ++i)
    difficulty[i] = 0;

  bits = Reverse32 (bits);

  char exponent = bits & 0xff;
  uint32_t significand = bits >> 8;

  for (int i = 0; i < 3; ++i) {
    // Endianness is reversed in this step
    unsigned char thisvalue = (unsigned char)(significand >> (8 * i));

    int index = 32 - exponent + i;
    difficulty[index / 4] = difficulty[index / 4] |
        ((unsigned int)thisvalue << (8 * (3 - (index % 4))));
  }
}

// Hashes block with given nonce, stores hash in result
hash256 hashblock (uint32_t nonce, char *version, char *prevhash, char *merkle_root, char *time, char *nbits) {
  uint32_t blockheader[20];

  hexstr_to_intarray (version, blockheader);
  hexstr_to_intarray (prevhash, blockheader + 1);
  hexstr_to_intarray (merkle_root, blockheader + 9);
  hexstr_to_intarray (time, blockheader + 17);
  hexstr_to_intarray (nbits, blockheader + 18);
  *(blockheader + 19) = nonce;

  // print_bytes((unsigned char*)blockheader, 80);

  for (int i = 0; i < 20; ++i)
    blockheader[i] = Reverse32 (blockheader[i]);

  hash256 r0 = sha256 ((char *)blockheader, 80);

  return sha256 (r0.b, 32);

  // print_bytes((unsigned char*)result, 32);
}

// Searches for a valid block hash with given nonce, given difficulty defined by nbits
// Returns nonce for which this block is valid
uint32_t mineblock (uint32_t noncestart, char *version, char *prevhash, char *merkle_root, char *time, char *nbits) {
  // First convert bits to a uint32_t, then convert this to a difficulty
  uint32_t difficulty[8];
  uint32_t bits[1];
  hexstr_to_intarray (nbits, bits);
  bits_to_difficulty (*bits, difficulty);

  uint32_t nonce = noncestart - 1;
  simple_timer_t counter_time;

  while (true) {
    nonce++;

    hash256 hash = hashblock (nonce, version, prevhash, merkle_root, time, nbits);
    
    if (memcmp(hash.b, difficulty, 32) < 0) {
      double hashrate = 10000000.0 / counter_time.end().to_sec();
      *output_file << "Speed " << hashrate << " hashes / sec" << std::endl;
    	return nonce;
    }
  }
  return 0;
}

bool mining_test () {
  *output_file << "Mining Test : not ready" << std::endl;
  /*
  // Genesis Block info
  char version[] = "01000000";
  char prevhash[] = "0000000000000000000000000000000000000000000000000000000000000000";
  char merkle_root[] = "3BA3EDFD7A7B12B27AC72C3E67768F617FC81BC3888A51323A9FB8AA4B1E5E4A";
  char time[] = "29AB5F49";
  char nbits[] = "FFFF001D";

  uint32_t nonce = mineblock (2083236890, version, prevhash, merkle_root, time, nbits);
  // uint32_t nonce = mineblock (10, version, prevhash, merkle_root, time, nbits);

  *output_file << "Nonce: " << (nonce == 2083236893 ? "Correct!" : "False") << std::endl;

  // hashblock((uint32_t)2083236893, result);
  hash256 result = hashblock (nonce, version, prevhash, merkle_root, time, nbits);

  for (int i = 0; i < 8; ++i)
    result.i[i] = Reverse32 (result.i[i]);

  print_bytes_reversed ((unsigned char *)result.b, 32);
*/
  return true;
}