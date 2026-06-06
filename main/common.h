/* *****************************************************************************
 * common.h v0.0.0000
 * 
 * Provide basic function and constant for all source code
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _COMMON_INCLUDED_
#define _COMMON_INCLUDED_

// ================================
//  Global Macro & Primitive Redefinition
// ================================
#if (defined(_MSC_VER) && _MSC_VER < 1600) /*|| defined(__SYMBIAN32__) */
  #include <assert.h>
  
  typedef          __int8   byte;
  typedef          __int16  shrt;
  typedef          __int32  int32;
  typedef          __int64  int64;
  typedef unsigned __int8 	ubyte;
  typedef unsigned __int16 	ushrt;
  typedef unsigned __int32 	uint32;
  typedef unsigned __int64 	uint64;
  typedef unsigned __int64 	iter;
  
  
  
  #define ASSERT(X)           assert(X)
#elif !defined(NO_STDC)
  #include <stdint.h>
  #include <stdlib.h>
  #include <assert.h>
  
  typedef int8_t    byte;
  typedef int16_t   shrt;
  typedef int32_t   int32;
  typedef int64_t   int64;
  typedef uint8_t 	ubyte;
  typedef uint16_t 	ushrt;
  typedef uint32_t 	uint32;
  typedef uint64_t 	uint64;
  typedef size_t    iter;
  
  #define ASSERT(X)           assert(X)
#else
  typedef char               byte;
  typedef short              shrt;
  typedef int                int32;
  typedef long long          int64;
  typedef unsigned char      ubyte;
  typedef unsigned short     ushrt;
  typedef unsigned int       uint32;
  typedef unsigned long long uint64;
  typedef unsigned int       iter;
  
  #error "need non-standart abort!"
  #define ASSERT(X)
  #define EXIT_SUCCESS  0
  #define EXIT_FAILURE -1
#endif

typedef unsigned int      uint;
typedef unsigned long     ulong;
typedef char *            String;

typedef struct { iter count; const char *cstr; } StringView;

// clang/gcc builtin feature
#ifndef __has_builtin
  // non clang/gcc always false
  #define __has_builtin(x) 0
#endif


#define TODO(X)               // Message that need todo in future: (X)
#define PRIVATE_STRINGIFY(X)  #X
#define STRINGIFY(X)          PRIVATE_STRINGIFY(X)
#define STACK_ARR_LEN(X)      (sizeof((X)) / sizeof((X)[0]))

#if defined(_MSC_VER)
  #if defined(_WIN32) || defined(WIN32)
    #ifndef _CRT_SECURE_NO_WARNINGS
      #define _CRT_SECURE_NO_WARNINGS
    #endif
    #ifndef _CRT_NONSTDC_NO_DEPRECATE
      #define _CRT_NONSTDC_NO_DEPRECATE
    #endif
  #endif

  #define CDECL            __cdecl
  #define UNUSED(x)        ((void)x)
  #define UNUSED_ARG(x)    __pragma(warning(suppress : 4100 4101)) x
#elif defined(__GNUC__)
  #define CDECL            /* no translate */
  #define UNUSED(x)        ((void)x)
  #define UNUSED_ARG(x)    __attribute__((unused)) x
#elif defined(__clang__)
  #define CDECL            /* no translate */
  #define UNUSED(x)        ((void)x)
  #define UNUSED_ARG(x)    __attribute__((unused)) x
  
#else /* Unknown compiler */
  #error "Not ready for this compiler"
#endif


#ifdef __cplusplus
  #define IS_ERROR(x) if (!!(x)) [[unlikely]]
  #define LIKELY(x)   (x) [[likely]]
  #define UNLIKELY(x) (x) [[unlikely]]
  #define EXPECT(x,y) ((x) == (y)) [[likely]]
#elif __has_builtin(__builtin_expect)
  #define IS_ERROR(x) if (__builtin_expect(!!(x), 0))
  #define LIKELY(x)   __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
  #define EXPECT(x,y) __builtin_expect((x), (y))
#else
  #define IS_ERROR(x) if (!!(x))
  #define LIKELY(x)   (x)
  #define UNLIKELY(x) (x)
  #define EXPECT(x,y) ((x) == (y))
#endif

#define MIN(X,Y)  (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y)  (((X) > (Y)) ? (X) : (Y))


#ifndef __cplusplus
  #define CAST(T) (T)
#else
  #define CAST(T) (decltype(T))
extern "C" {
#endif // __cplusplus

#ifdef _WIN32
int convert_wchar_to_utf8(char *, iter, const wchar_t *);
#endif // _WIN32

// assembly convetion
void hello_world ();

/* ================================
 *  Standar Utility Functions
 *  just bridge of memory access + modification
 * ================================
 */
void *util_malloc (iter);
void *util_calloc (iter,iter);
void *util_realloc(void*,iter);
void  util_memfree(void*);
void  util_memswap(void*,void*,iter);
void  util_memflip(void*,iter);
void  util_memcpy (void*,const void*,iter);
int   util_memcmp (const void*,const void*,iter);
void  util_memmove(void*,void*,iter);
void  util_memset (void*,int,iter);
iter  util_strlen (const char*);
iter  util_clz(ulong);
iter  util_bitlead(ulong);

/* ================================
 *  String Functions
 * ================================
 */
void string_append_char(String*,char);
void string_append_cstr(String*,const char *, iter);
void string_append     (String*,const char *, ...);
void string_reserve    (String*,iter);
iter string_len        (const String);
void string_clean      (String);
void string_free       (String*);

/* ================================
 *  String View Functions
 * ================================
 */
// generate StringView from String
StringView stringview_str       (const String);
// chop by whitespace
StringView stringview_chop      (const String);
StringView stringview_chop_char (const String,const char);
StringView stringview_chop_chars(const String,const char*);
StringView stringview_chop_cstr (const String,const char*);
// StringView helper
int        stringview_equal     (const StringView,const StringView);


/* ================================
 *  IMath Functions
 * ================================
 */
uint   imath_iabs  (int);
float  imath_fabs  (float);
float  imath_fmax  (float, float);
float  imath_fmin  (float, float);
float  imath_floor (float);
float  imath_frexp (float,int*);
float  imath_sin   (float);
float  imath_cos   (float);
float  imath_tan   (float);
float  imath_asin  (float);
float  imath_acos  (float);
float  imath_atan  (float);
float  imath_atan2 (float,float);
float  imath_pow   (float,float);
float  imath_log   (float);
float  imath_log2  (float);
float  imath_exp   (float);
float  imath_exp2  (float);
float  imath_len   (const float*,iter);
float  imath_hypot (float,float);
float  imath_sqrt  (float);
float  imath_isqrt (float);
ubyte  imath_flip8 (ubyte);
ushrt  imath_flip16(ushrt);
uint32 imath_flip32(uint32);
uint64 imath_flip64(uint64);
int32  imath_rotl32(int32, const iter);
int64  imath_rotl64(int64, const iter);
int32  imath_rotr32(int32, const iter);
int64  imath_rotr64(int64, const iter);

#define RAND_CAST(T) T imath_rand_##T ();
RAND_CAST(byte);
RAND_CAST(shrt);
RAND_CAST(int32);
RAND_CAST(int64);
RAND_CAST(ubyte);
RAND_CAST(ushrt);
RAND_CAST(uint32);
RAND_CAST(uint64);
#undef RAND_CAST
float imath_rand_float();

#ifdef __cplusplus
}
#endif

/* ================================
 *  dynamic array 
 *  Apply in macro
 *  ARRAY object format at least contain this
 * struct {
 *   T *items;
 *   iter cap;
 *   iter count;
 * } T's;
 *  
 * ================================
 */
#define da_roundSize   8
#define da_roundMask   7
#define da_reserve(a, need) do { \
  if ((need) <= (a)->cap) break; \
  (a)->cap = ((need) & ~da_roundMask) + da_roundSize * !!((need) & da_roundMask); \
  (a)->items = util_realloc((a)->items, (a)->cap * sizeof(*(a)->items)); \
  ASSERT((a)->items && "Fail to allocate heap"); \
} while (0)
#define da_atleast(a, z) do { \
  if ((a)->count >= (z)) break;\
  da_reserve((a), (z)); \
  util_memset((a)->items + (a)->count, 0, ((z)-(a)->count) * sizeof(*(a)->items)); \
  (a)->count = (z); \
} while (0)
#define da_copy(a,b) do {\
  da_reserve((a),(b)->count);\
  util_memcpy((a)->items, (b)->items, (b)->count * sizeof(*(a)->items)); \
  (a)->count = (b)->count; \
} while (0)
#define da_append(a, item) do { \
  da_reserve((a), (a)->count + 1); \
  (a)->items[(a)->count++] = (item); \
} while (0)
#define da_free(a) do {\
  if ((a)->items) util_memfree((a)->items);\
  util_memset((a), 0, sizeof(*a)); \
} while (0)
#define da_clean(a) (a)->count = 0
#define da_appends(a, b, l) do { \
  da_reserve((a), (a)->count + (l)); \
  util_memcpy((a)->items + (a)->count, (b), (l)*sizeof(*(a)->items)); \
  (a)->count += (l); \
} while (0)
#define da_index(a,i) (a)->items[(ASSERT((i) < (a)->count), (i))]
#define da_last(a) (a)->items[(ASSERT((a)->count), (a)->count - 1)]
#define da_pop(a) (a)->items[(ASSERT((a)->count), --(a)->count)]
#define da_unorder_remove(a, j) (a)->items[(j)] = (a)->items[(ASSERT((j) < (a)->count), --(a)->count)]
#define da_remove(a, j) do { \
  ASSERT((j) < (a)->count); \
  if ((j) < --(a)->count) \
    util_memcpy(&(a)->items[(j)], &(a)->items[(j) + 1], (a)->count - (j));\
} while(0)
#define da_foreach(T, it, a) for (T *it = (a)->items, *__end = (a)->items + (a)->count; it < __end; ++it)
#define da_rforeach(T, it, a) for (T *it = (a)->items + (a)->count; (it--) > (a)->items; )




#endif // _COMMON_INCLUDED_
