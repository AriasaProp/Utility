#include "algorithm/hash.h"
#include "util/console_out.h"
#include "util/profiling.h"


int main(int UNUSED_ARG(c), char **UNUSED_ARG(v)) {
  PRINT_INF("Hash test -> ");
  pr_time start = profiling_current_time();
  iter i, j, n;
  String str = NULL;
  int ret = EXIT_FAILURE;
  ubyte *resA = CAST(ubyte*)util_malloc(HASH512_IN_BYTES * 2);
  if (!resA) {
    PRINT_ERR("Memory allocation fail!");
    goto main_ret;
  }
  const char *input[] = {
    NULL,
    "Hello World",
    "The quick brown fox jumps over the lazy dog",
  };
  ubyte *resB = resA + HASH512_IN_BYTES;
  {
    const char * const output[] = {
      "D41D8CD98F00B204E9800998ECF8427E",
      "B10A8DB164E0754105B7A99BE72E3FE5",
      "9E107D9D372BB6826BD81D3542A419D6",
    };
    for (i = 0, j = STACK_ARR_LEN(input), n = HASH128_IN_BYTES; i < j; ++i) {
      hash_md5(CAST(uint32*)resA, input[i], util_strlen(input[i]));
      hash_cstr_to_ubyte(resB, output[i], n);
      if (util_memcmp(resA, resB, n)) {
        string_append(&str, "md5");
        goto main_err;
      }
    }
  }
  {
    const char *const output[] = {
      "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709",
      "0A4D55A8D778E5022FAB701977C5D840BBC486D0",
      "2FD4E1C67A2D28FCED849EE1BB76E7391B93EB12",
    };
    for (i = 0, j = STACK_ARR_LEN(input), n = HASH160_IN_BYTES; i < j; ++i) {
      hash_sha1(CAST(uint32*)resA, input[i], util_strlen(input[i]));
      hash_cstr_to_ubyte(resB, output[i], n);
      if (util_memcmp(resA, resB, n)) {
        string_append(&str, "sha1");
        goto main_err;
      }
    }
  }
  {
    const char *const output[] = {
      "D14A028C2A3A2BC9476102BB288234C415A2B01F828EA62AC5B3E42F",
      "C4890FAFFDB0105D991A461E668E276685401B02EAB1EF4372795047",
      "730E109BD7A8A32B1CB9D9A09AA2325D2430587DDBC0C38BAD911525",
    };
    for (i = 0, j = STACK_ARR_LEN(input), n = HASH224_IN_BYTES; i < j; ++i) {
      hash_sha224(CAST(uint32*)resA, input[i], util_strlen(input[i]));
      hash_cstr_to_ubyte(resB, output[i], n);
      if (util_memcmp(resA, resB, n)) {
        string_append(&str, "sha224");
        goto main_err;
      }
    }
  }
  {
    const char *const output[] = {
      "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855",
      "A591A6D40BF420404A011733CFB7B190D62C65BF0BCDA32B57B277D9AD9F146E",
      "D7A8FBB307D7809469CA9ABCB0082E4F8D5651E46D3CDB762D02D0BF37C9E592",
    };
    for (i = 0, j = STACK_ARR_LEN(input), n = HASH256_IN_BYTES; i < j; ++i) {
      hash_sha256(CAST(uint32*)resA, input[i], util_strlen(input[i]));
      hash_cstr_to_ubyte(resB, output[i], n);
      if (util_memcmp(resA, resB, n)) {
        string_append(&str, "sha256");
        goto main_err;
      }
    }
  }
  {
    const char *const output[] = {
      "38B060A751AC96384CD9327EB1B1E36A21FDB71114BE07434C0CC7BF63F6E1DA274EDEBFE76F65FBD51AD2F14898B95B",
      "99514329186B2F6AE4A1329E7EE6C610A729636335174AC6B740F9028396FCC803D0E93863A7C3D90F86BEEE782F4F3F",
      "CA737F1014A48F4C0B6DD43CB177B0AFD9E5169367544C494011E3317DBF9A509CB1E5DC1E85A941BBEE3D7F2AFBC9B1",
    };
    for (i = 0, j = STACK_ARR_LEN(input), n = HASH384_IN_BYTES; i < j; ++i) {
      hash_sha384(CAST(uint64*)resA, input[i], util_strlen(input[i]));
      hash_cstr_to_ubyte(resB, output[i], n);
      if (util_memcmp(resA, resB, n)) {
        string_append(&str, "sha384");
        goto main_err;
      }
    }
  }
  {
    const char *const output[] = {
      "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E",
      "2C74FD17EDAFD80E8447B0D46741EE243B7EB74DD2149A0AB1B9246FB30382F27E853D8585719E0E67CBDA0DAA8F51671064615D645AE27ACB15BFB1447F459B",
      "07E547D9586F6A73F73FBAC0435ED76951218FB7D0C8D788A309D785436BBB642E93A252A954F23912547D1E8A3B5ED6E1BFD7097821233FA0538F3DB854FEE6",
    };
    for (i = 0, j = STACK_ARR_LEN(input); i < j; ++i) {
      hash_sha512(CAST(uint64*)resA, input[i], util_strlen(input[i]));
      hash_cstr_to_ubyte(resB, output[i], HASH512_IN_BYTES);
      if (util_memcmp(resA, resB, HASH512_IN_BYTES)) {
        string_append(&str, "sha512");
        goto main_err;
      }
    }
  }
  ret = EXIT_SUCCESS;
  string_clean(str);
  profiling_append_as_time2(&str, profiling_time_since(start));
  printf(GREEN"Success"RESET" %s\n", str);
  goto main_ret;
main_err:
  printf(RED"Err %s \n"RESET, str);
  string_clean(str);
  if (i) PRINT_ERR("NULL");
  else PRINT_ERR("\"%s\"", input[i]);
  string_clean(str);
  hash_ubyte_append_string(&str, resA, n);
  printf("->%s\n", str);
  string_clean(str);
  hash_ubyte_append_string(&str, resB, n);
  PRINT_ERR("Expt:%s\n", str);
main_ret:
  util_memfree(resA);
  string_free(&str);
  return ret;
}

