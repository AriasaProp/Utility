#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "hash.hpp"

// util
void print_bytes (const unsigned char *data, size_t dataLen, bool format = true) {
  for (size_t i = 0; i < dataLen; ++i) {
    std::cout << std::hex << std::setw (2) << (int)data[i];
    if (format) {
      std::cout << (((i + 1) % 16 == 0) ? "\n" : " ");
    }
  }
  std::cout << std::endl;
}
void print_bytes_reversed (const unsigned char *data, size_t dataLen, bool format = true) {
  for (size_t i = dataLen; i > 0; i--) {
    std::cout << std::hex << std::setw (2) << (int)data[i - 1];
    if (format) {
      std::cout << (((i - 1) % 16 == 0) ? "\n" : " ");
    }
  }
  std::cout << std::endl;
}
uint32_t Reverse32 (uint32_t value) {
  return (((value & 0x000000FF) << 24) |
          ((value & 0x0000FF00) << 8) |
          ((value & 0x00FF0000) >> 8) |
          ((value & 0xFF000000) >> 24));
}
unsigned char *hexstr_to_char (const char *hexstr) {
  size_t len = strlen (hexstr);
  size_t final_len = len / 2;
  unsigned char *chars = (unsigned char *)malloc ((final_len + 1));
  for (size_t i = 0, j = 0; j < final_len; i += 2, j++)
    chars[j] = (hexstr[i] % 32 + 9) % 25 * 16 + (hexstr[i + 1] % 32 + 9) % 25;
  chars[final_len] = '\0';
  return chars;
}
void hexstr_to_intarray (const char *hexstr, uint32_t *outputloc) {
  size_t len = strlen (hexstr);
  size_t intlen = (len + 7) / 8; // +7 ensures that we do a ceiling divide
  unsigned char *bytes = hexstr_to_char (hexstr);

  for (size_t i = 0; i < intlen; i++) {
    uint32_t a = (uint32_t)bytes[i * 4 + 3] << 24;
    *(outputloc + i) = ((uint32_t)bytes[i * 4]) + ((uint32_t)bytes[i * 4 + 1] << 8) + ((uint32_t)bytes[i * 4 + 2] << 16) + ((uint32_t)bytes[i * 4 + 3] << 24);
  }
}
// util end

// miner

// Converts bits to a 256-bit value that we can compare our hash against
void bits_to_difficulty (uint32_t bits, uint32_t *difficulty) {
  for (int i = 0; i < 8; i++)
    difficulty[i] = 0;

  bits = Reverse32 (bits);

  char exponent = bits & 0xff;
  uint32_t significand = bits >> 8;

  for (int i = 0; i < 3; i++) {
    // Endianness is reversed in this step
    unsigned char thisvalue = (unsigned char)(significand >> (8 * i));

    int index = 32 - exponent + i;
    difficulty[index / 4] = difficulty[index / 4] |
        ((unsigned int)thisvalue << (8 * (3 - (index % 4))));
  }
}

// Hashes block with given nonce, stores hash in result
void hashblock (uint32_t nonce, char *version, char *prevhash, char *merkle_root, char *time, char *nbits, uint32_t *result) {
  uint32_t blockheader[20];

  hexstr_to_intarray (version, blockheader);
  hexstr_to_intarray (prevhash, blockheader + 1);
  hexstr_to_intarray (merkle_root, blockheader + 9);
  hexstr_to_intarray (time, blockheader + 17);
  hexstr_to_intarray (nbits, blockheader + 18);
  *(blockheader + 19) = nonce;

  // print_bytes((unsigned char*)blockheader, 80);

  for (int i = 0; i < 20; i++)
    blockheader[i] = Reverse32 (blockheader[i]);

  uint32_t hash0[8];
  sha256 (blockheader, 640, hash0);

  sha256 (hash0, 256, result);

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

  char solved = 0;
  uint32_t hash[8];
  uint32_t nonce = noncestart - 1;

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now ();

  while (true) {
    nonce++;

    hashblock (nonce, version, prevhash, merkle_root, time, nbits, hash);

    for (int i = 0; i < 8; i++) {
      if (hash[7 - i] < difficulty[i]) {
        solved = 1;
        return nonce;
      } else if (hash[7 - i] > difficulty[i])
        break;
      // And if they're equal, we keep going!
    }

    if (((nonce - noncestart) % 10000) == 0 && nonce != noncestart) {
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
      long duration = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count ();

      float hashrate = 10000000.0 / (float)duration;
      std::cout << "Currently mining at " << hashrate << " hashes / second" << std::endl;
      start = std::chrono::steady_clock::now ();
    }
  }
}

bool Mining_test () {

  // Genesis Block info
  char version[] = "01000000";
  char prevhash[] = "0000000000000000000000000000000000000000000000000000000000000000";
  char merkle_root[] = "3BA3EDFD7A7B12B27AC72C3E67768F617FC81BC3888A51323A9FB8AA4B1E5E4A";
  char time[] = "29AB5F49";
  char nbits[] = "FFFF001D";

  uint32_t result[8];
  uint32_t nonce = mineblock (2083236890, version, prevhash, merkle_root, time, nbits);
  // uint32_t nonce = mineblock (10, version, prevhash, merkle_root, time, nbits);

  std::cout << "Block solved ! Nonce: " << nonce << std::endl;
  std::cout << "Block hash:" << std::endl;

  // hashblock((uint32_t)2083236893, result);
  hashblock (nonce, version, prevhash, merkle_root, time, nbits, result);

  for (int i = 0; i < 8; i++)
    result[i] = Reverse32 (result[i]);

  print_bytes_reversed ((unsigned char *)result, 32);

  // test sha256
  uint32_t result[8]{};
  const char *input[2][2]{
      {"abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},
      {"Hello World!", "7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069"}};
  for (const auto &i : input) {
    sha256 ((const uint32_t *)i[0], strlen (i[0]) * 8, result);
    if (memcmp (result, i[1], 32)) return false;
  }
  std::cout << "sha256 hash is good!" << std::endl;

  return true;
}