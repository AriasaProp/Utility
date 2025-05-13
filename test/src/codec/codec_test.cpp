extern char *data_address;
extern void hasher_test();

#include <cstring>

void codec_test() {
  strcpy(data_address, "data/codec");
  hasher_test();
}