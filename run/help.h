#ifndef help_msg
#define help_msg "\n"\
  "Usage: run/nob <command> [group|kind] [flags]\n"\
  "   COMMAND\tDescription\n"\
  "h[elp]     show this message.\n"\
  "c[lean]    clean generated binary [group] files/folders or all.\n"\
  "s[tatus]   show current device [group] or all.\n"\
  "i[nfo]   	show device information.\n"\
  "test       make and run test by [group] or all.\n"\
  "benchmark  make and run benchmark by [group] or all.\n"\
  "\n"\
  "   GROUP\n"\
  "test       test [group]\n"\
  "benchmark  benchmark [group]\n"\
  "\n"\
  "   FLAGS\n"\
  "-b,--build Keep build exec, even already updated.\n"\
  "-d,--debug Run exec in debug mode. always removed after.\n"
#endif // help_msg