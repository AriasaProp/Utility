#ifndef NOB_EXTRA_DEFINITION
#define NOB_EXTRA_DEFINITION
#include "nob.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define nob_da_remove_first_item(da) if ((da)->count) memcpy((da)->items, (da)->items + 1, ((da)->count -= 1) * sizeof(*(da)->items))

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

#ifndef NOB_EXTRA_STRIP_PREFIX_GUARD_
#define NOB_EXTRA_STRIP_PREFIX_GUARD_
  #ifndef NOB_UNSTRIP_PREFIX
  	#define mkdir_rec nob_mkdir_rec
  	#define da_remove_first_item nob_da_remove_first_item
	#endif
#endif

#endif // NOB_EXTRA_DEFINITION
