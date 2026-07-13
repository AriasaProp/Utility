#ifndef _STB_LOCAL_INCLUDED_
#define _STB_LOCAL_INCLUDED_

#define PNG_SIGNATURE "\x89PNG\r\n\x1A\n"
#define MAX_DIMENSIONS (1 << 24)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void stb_clean_error();
const char *stb_get_error();
void stb_set_error(const char*,...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _STB_LOCAL_INCLUDED_