#ifndef HELP
#define HELP
statics const char *help_msg = "\n"
  "Usage: ./nob <command> <flags>\n"
  "\tCOMMAND\t\t\tDescription\n"
  "h[elp]\t\tshow this message.\n"
  "t[ests]\t\tmake and run all test.\n"
  "c[lean]\t\tclean generated binary files/folders.\n"
  "s[tatus]\tshow current device stat.\n"
  "\n"
  "rand_test\trun random test.\n"
  "matrix_test\trun matrix test.\n"
  "complex_test\trun complex test.\n"
  "bigInteger_test\trun bigInteger test.\n"
  "hash_test\trun hash test.\n"
  "sort_test\trun sort test.\n"
  "\tFLAGS\t\t\tDescription\n"
  "-b,--build\tForce command to keep build even already exists.\n"
  "          \tonly works on tests.\n";
#endif HELP