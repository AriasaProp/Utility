#ifndef HELP
#define HELP
#define help_msg "\n"\
  "Usage: ./nob <command> <flags>\n"\
  "\tCOMMAND\t\t\tDescription\n"\
  "h[elp]\t\tshow this message.\n"\
  "t[ests]\t\tmake and run all test.\n"\
  "c[lean]\t\tclean generated binary files/folders.\n"\
  "s[tatus]\tshow current device stat.\n"\
  "\n"\
  "rand_test\trun random test.\n"\
  "matrix_test\trun matrix test.\n"\
  "complex_test\trun complex test.\n"\
  "bigInteger_test\trun bigInteger test.\n"\
  "hash_test\trun hash test.\n"\
  "sort_test\trun sort test.\n"\
  "\tFLAGS\t\t\tDescription\n"\
  "-b,--build\tKeep build exec, even already updated.\n"\
  "-d,--debug\tRun exec in debug mode. always removed after.\n"
#endif // HELP