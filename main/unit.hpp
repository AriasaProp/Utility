#ifndef UNIT_INCLUDED
#define UNIT_INCLUDED

/*
 *  Unit Header
 *
 *  Provide basic unit rename or object struct
 *
 *
 *
 */


#if (defined(_MSC_VER) && _MSC_VER < 1600) /*|| defined(__SYMBIAN32__) */
using int8_t = __int8;
using int16_t = __int16;
using int32_t = __int32;
using int64_t = __int64;
using uint8_t = unsigned __int8;
using uint16_t = unsigned __int16;
using uint32_t = unsigned __int32;
using uint64_t = unsigned __int64;
using size_t = unsigned __int64;
#else
#include <cstdint>
#endif

#endif // UNIT_INCLUDED