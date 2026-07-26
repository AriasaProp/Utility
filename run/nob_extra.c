#include "nob_extra.h"
#include "nob.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

bool nob_mkdir_rec(const char *path) {
#ifdef _WIN32
  int result = _mkdir(path);
#else
  int result = mkdir(path, 0755);
#endif
  if (result < 0) {
    switch (errno) {
    case EEXIST:
#ifndef NOB_NO_ECHO
      nob_log(NOB_INFO, "directory `%s` already exists", path);
#endif // NOB_NO_ECHO
      return true;
    case ENOTDIR:
    case ENOENT: {
    	size_t point = nob_temp_save();
      bool res = nob_mkdir_rec(nob_temp_dir_name(path));
      res &= nob_mkdir_if_not_exists(path);
      nob_temp_rewind(point);
      return res;
    }
    default:
      nob_log(NOB_ERROR, "could not create directory `%s`: %s", path, strerror(errno));
      return false;
    }
  }
  return true;
}
