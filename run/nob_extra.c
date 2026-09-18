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
  int result =
#ifdef _WIN32
  	_mkdir(path);
#else
		mkdir(path, 0755);
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
    	size_t point = temp_save();
      bool res = nob_mkdir_rec(nob_temp_dir_name(path));
      res &= nob_mkdir_rec(path);
      temp_rewind(point);
      return res;
    }
    default:
      nob_log(NOB_ERROR, "could not create directory `%s`: %s", path, strerror(errno));
      return false;
    }
  }
  return true;
}

long double nob_nanos_unit(int64_t atime, char *c1, char *c2) {
  long double cal = NOB_DECLTYPE_CAST(long double)atime;
  size_t i;
  for (i = 0; cal >= 1000.0 && i < 3; ++i) {
    cal /= 1000.0;
  }
  *c2 = 's';
  *c1 = "num "[i];
  if (i > 3) {
    for (i = 0; cal >= 60.0 && i < 3; ++i) {
      cal /= 60.0;
    }
    *c2 = "sMHD"[i];
  }
  return cal;
}

#ifndef NOB_EXTRA_STRIP_PREFIX_GUARD_
#define NOB_EXTRA_STRIP_PREFIX_GUARD_
  #ifndef NOB_UNSTRIP_PREFIX
  	#define nanos_unit nob_nanos_unit
  	#define mkdir_rec nob_mkdir_rec
  	#define da_remove_first_item nob_da_remove_first_item
	#endif
#endif

#endif // NOB_EXTRA_DEFINITION
