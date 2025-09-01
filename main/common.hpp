/*
 *  Common Header
 *
 *  Provide basic function and constant for all source code
 *
 *
 *
 */

#ifndef COMMON_INCLUDED_
#define COMMON_INCLUDED_

#include <cstddef>
#include <cstdlib>

// ================================
//  Global Macro
// ================================

#if defined(_MSC_VER)

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#endif

#define INLINE        __forceinline
#define CDECL         __cdecl
#define UNUSED(x)     ((void)x)
#define UNUSED_ARG(x) __pragma(warning(suppress : 4100 4101)) x

#elif defined(__GNUC__) || defined(__clang__)
#define INLINE        inline
#define CDECL         /* no translate */
#define UNUSED(x)     ((void)x)
#define UNUSED_ARG(x) __attribute__((unused)) x
#else /* Unknown compiler */
#error "Not ready for this compiler"
#endif

#define MAX(X, Y)      ((X > Y) ? (X) : (Y))
#define MIN(X, Y)      ((X < Y) ? (X) : (Y))
// x min wall, y args, z max wall
#define CLAMP(X, Y, Z) ((X < Z) ? ((Y > Z) ? (Z) : (Y)) : (X))

#endif // COMMON_INCLUDED_