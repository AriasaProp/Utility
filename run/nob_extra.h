#ifndef NOB_EXTRA_DEFINITION
#define NOB_EXTRA_DEFINITION
#include <stdbool.h>

#define nob_da_remove_first_item(da) if ((da)->count) memcpy((da)->items, (da)->items + 1, ((da)->count -= 1) * sizeof(*(da)->items))

bool nob_mkdir_rec(const char *);

#ifndef NOB_EXTRA_STRIP_PREFIX_GUARD_
#define NOB_EXTRA_STRIP_PREFIX_GUARD_
  #ifndef NOB_UNSTRIP_PREFIX
  	#define mkdir_rec nob_mkdir_rec
  	#define da_remove_first_item nob_da_remove_first_item
	#endif
#endif

#endif // NOB_EXTRA_DEFINITION
