#include "codec/hash.h"
#include "util/console_out.h"

struct {
  const char *str,*hex;
} tests[] = {
  {.str = " "           , .hex = "36A9E7F1C95B82FFB99743E0C5C4CE95D83C9A430AAC59F84EF3CBFAB6145068"},
  {.str = "Hello World!", .hex = "7F83B1657FF1FC53B92DC18148A1D65DFC2D4B1FA3D677284ADDD200126D9069"},
};

int main(int UNUSED_ARG(c), char **UNUSED_ARG(v)) {
  iter i, j;
  String str = NULL;
  int ret = EXIT_FAILURE;
  hash256 hs = {0}, hi = {0};
  PRINT_INF("Try sha256 \n");
  for (i = 0, j = STACK_ARR_LEN(tests); i < j; ++i) {
    hash_cstr_to_h256(&hi, tests[i].hex);
    hash_sha256(&hs, tests[i].str, util_strlen(tests[i].str));
    if (util_memcmp(hs.b, hi.b, HASH256_IN_BYTES)) {
      PRINT_ERR("Inpt: \"%s\"\n", tests[i].str);
      string_clean(str);
      hash_h256_append_string(&str, hs);
      PRINT_ERR("Outp: %s\n", str);
      string_clean(str);
      hash_h256_append_string(&str, hi);
      PRINT_ERR("Expt: %s\n", str);
      //goto main_ret;
    }
  }
  ret = EXIT_SUCCESS;
//main_ret:
  string_free(&str);
  return ret;
}

