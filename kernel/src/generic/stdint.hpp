#pragma once

// Detect architecture
#if defined(__x86_64__) || defined(_M_X64)
#define X86_64
#else
#define X86_32
#endif

// 1. Exact-width integer types
#ifdef X86_64
typedef signed char         int8_t;
typedef unsigned char       uint8_t;

typedef short               int16_t;
typedef unsigned short      uint16_t;

typedef int                 int32_t;
typedef unsigned int        uint32_t;

typedef long                int64_t;      // 64-bit long
typedef unsigned long       uint64_t;

#else // X86_32
typedef signed char         int8_t;
typedef unsigned char       uint8_t;

typedef short               int16_t;
typedef unsigned short      uint16_t;

typedef int                 int32_t;
typedef unsigned int        uint32_t;

typedef long long           int64_t;      // 64-bit long long
typedef unsigned long long  uint64_t;
#endif

// 2. Least-width integer types
typedef int8_t   int_least8_t;
typedef uint8_t  uint_least8_t;

typedef int16_t  int_least16_t;
typedef uint16_t uint_least16_t;

typedef int32_t  int_least32_t;
typedef uint32_t uint_least32_t;

typedef int64_t  int_least64_t;
typedef uint64_t uint_least64_t;

typedef uint64_t size_t;

// 3. Fast integer types
#ifdef X86_64
typedef int8_t   int_fast8_t;
typedef uint8_t  uint_fast8_t;

typedef int16_t  int_fast16_t;
typedef uint16_t uint_fast16_t;

typedef int32_t  int_fast32_t;
typedef uint32_t uint_fast32_t;

typedef int64_t  int_fast64_t;
typedef uint64_t uint_fast64_t;

#else // X86_32
typedef int      int_fast8_t;
typedef unsigned int uint_fast8_t;

typedef int      int_fast16_t;
typedef unsigned int uint_fast16_t;

typedef int      int_fast32_t;
typedef unsigned int uint_fast32_t;

typedef long long int_fast64_t;
typedef unsigned long long uint_fast64_t;
#endif

// 4. Maximum-width integer types
#ifdef X86_64
typedef long        intmax_t;
typedef unsigned long uintmax_t;
#else
typedef long long   intmax_t;
typedef unsigned long long uintmax_t;
#endif

// 5. Pointer-sized integer types
#ifdef X86_64
typedef long        intptr_t;
typedef unsigned long uintptr_t;
#else
typedef int         intptr_t;
typedef unsigned int uintptr_t;
#endif

// 6. Other standard defines (optional)
#define INT8_MAX   127
#define INT8_MIN   (-128)
#define UINT8_MAX  255

#define INT16_MAX  32767
#define INT16_MIN  (-32768)
#define UINT16_MAX 65535

#define INT32_MAX  2147483647
#define INT32_MIN  (-2147483648)
#define UINT32_MAX 4294967295U

#ifdef X86_64
#define INT64_MAX  9223372036854775807L
#define INT64_MIN  (-9223372036854775807L-1)
#define UINT64_MAX 18446744073709551615UL
#else
#define INT64_MAX  9223372036854775807LL
#define INT64_MIN  (-9223372036854775807LL-1)
#define UINT64_MAX 18446744073709551615ULL
#endif
