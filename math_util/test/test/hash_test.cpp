#include "hash.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

extern std::ostream output_file;
extern char text_buffer[2048];

bool hash_test (const char *data) {
  output_file << "Hash Test" << std::endl;
  bool passed = true;
  size_t i, j, k, l, m, n;

  output_file << "SHA256: " << std::endl;
  try {
    strcpy (text_buffer, data);
    strcat (text_buffer, "/data/sha256.txt");
    std::ifstream file (text_buffer);
    if (!file.is_open ()) [[unlikely]]
      throw "file data not found!";
    try {
      std::string line, first, second;
      while (std::getline (file, line)) {
        std::istringstream ss (line);
        if (std::getline (ss, first, ',') && std::getline (ss, second, ',')) {
          hash256 o = sha256 (second.c_str (), second.size ());
          hash256 e;
          for (i = 0, k = 0; i < 8; ++i, ++k) {
            for (j = 0; j < 8; ++j) {
              e.i[i] <<= 4;
              e.i[i] = second[k] - 48 + (second[k] >= 'a') * 39;
            }
          }
          if (memcmp (o.b, e.b, 64)) throw "wrong sha256 hashing result";
        }
      }
    } catch (const char *err) {
      file.close ();
      throw err;
    }
    file.close ();
  } catch (const char *err) {
    passed &= false;
    output_file << err << std::endl;
  }

  output_file << "RandomX: Not yet." << std::endl;

  output_file << "Hash Test End" << std::endl;

  return passed;
}