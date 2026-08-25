#ifndef info_msg
  // os info
#if defined(_WIN64)
#  define __info_os_ "Windows(64bit)"
#elif defined(_WIN32)
#  define __info_os_ "Windows(32bit)"
#elif defined(__linux__)
#  define __info_os_ "Linux"
#elif defined(__APPLE__) && defined(__MACH__)
#  define __info_os_ "MacOS"
#elif defined(__unix__)
#  define __info_os_ "Unix"
#else
#  define __info_os_ "Unknown"
#endif
  // compiler info
#if defined(__clang__)
#  define __info_compiler "Clang %d.%d.%d"
#  define __info_compiler_args __clang_major__, __clang_minor__, __clang_patchlevel__
#elif defined(__GNUC__) || defined(__GNUG__)
#  define __info_compiler "GCC %d.%d.%d"
#  define __info_compiler_args __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#elif defined(_MSC_VER)
#  define __info_compiler "MSVC %d"
#  define __info_compiler_args _MSC_VER
#else
#  define __info_compiler "Unknown"
#  define __info_compiler_args
#endif
  // arch info
#if defined(__x86_64__) || defined(_M_X64)
#  define __info_arch_ "x86_64(64bit)"
#elif defined(__i386) || defined(_M_IX86)
#  define __info_arch_ "x86 (32bit)"
#elif defined(__aarch64__)
#  define __info_arch_ "ARM64 (64bit)"
#elif defined(__arm__) || defined(_M_ARM)
#  define __info_arch_ "ARM (32bit)"
#else
#  define __info_arch_ "Unknown"
#endif
  // Byte order info
#ifdef BYTE_ORDER
#  if BYTE_ORDER == LITTLE_ENDIAN
#    define __info_bo_ "Little-Endian (BYTE_ORDER)"
#  else
#    define __info_bo_ "Big-Endian (BYTE_ORDER)"
#  endif // BYTE_ORDER
#elif defined(__BYTE_ORDER__)
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define __info_bo_ "Little-Endian (__BYTE_ORDER__)"
#  else
#    define __info_bo_ "Big-Endian (__BYTE_ORDER__)"
#  endif // __BYTE_ORDER__
#else
static unsigned int __info_x = 1;
#  define __info_bo_ ((*(char*)&__info_x == 1) ? "Little-Endian (Manual)" : "Big-Endian (Manual)")
#endif

static const char *__info_simd_ = 
#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
#  define USE_SIMD
    "AVX"
#endif
#ifdef __SSE__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "SSE"
#endif
#ifdef __SSE2__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "SSE2"
#endif
#ifdef __SSE3__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "SSE3"
#endif
#ifdef __SSE4_1__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "SSE4_1"
#endif
#ifdef __SSE4_2__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "SSE4_2"
#endif
#if defined(__ARM_NEON) || defined(__NEON__)
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "NEON"
#endif
#ifdef _MSC_VER
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "MSVC"
#endif
#ifdef __MMX__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "MMX"
#endif
#ifdef __ALTIVEC__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "ALTIVEC"
#endif
#ifdef __VSX__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "VSX"
#endif
#ifdef __RISC_V_VECTOR__
#  ifndef USE_SIMD
#    define USE_SIMD
#  else
    ", "
#  endif
    "RISC_V_VECTOR"
#endif
#ifndef USE_SIMD
    "NO_SIMD"
#else
#  undef USE_SIMD
#endif
;
static const char *__info_osup_ = 
#ifdef __SIZEOF_INT128__
  "int128"
#endif
;

#define info_msg \
  "      [System build info]   \n" \
  " Operating System : " __info_os_     "\n" \
  " Compiler         : " __info_compiler"\n" \
  " Architecture     : " __info_arch_   "\n" \
  " Byte-Order       : " __info_bo_     "\n" \
  " SIMD support     : %s\n" \
  " Other Supports   : %s\n" \
  , __info_compiler_args , __info_simd_, __info_osup_


#endif // info_msg