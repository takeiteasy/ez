//
//  ez128.h
//  ez
//
//  Created by George Watson on 19/12/2023.
//
// Based off https://codereview.stackexchange.com/questions/282353/128-bit-integer-type-in-gcc-clang
// This exists to copy and paste

#ifndef EZINT128_HEADER
#define EZINT128_HEADER

#include <stdint.h>

#if defined(__clang__)
#define EZ_COMPILER clang
#elif defined(__GNUC__) || defined(__GNUG__)
#define EZ_COMPILER GCC
#else
#error "Unknown compiler"
#endif

#pragma EZ_COMPILER diagnostic push
#pragma EZ_COMPILER diagnostic ignored "-Wpragmas"
#pragma EZ_COMPILER diagnostic ignored "-Wpedantic"
#pragma EZ_COMPILER diagnostic ignored "-pedantic"
typedef unsigned __int128 uint128_t;
#pragma EZ_COMPILER diagnostic pop

#if !defined(UINT128)
#define UINT128(n) ((uint128_t)n)
#endif
#if !defined(UINT128_MAX)
#define UINT128_MAX (~UINT128(0))
#endif

#pragma EZ_COMPILER diagnostic push
#pragma EZ_COMPILER diagnostic ignored "-Wpragmas"
#pragma EZ_COMPILER diagnostic ignored "-Wpedantic"
#pragma EZ_COMPILER diagnostic ignored "-pedantic"
typedef __int128 int128_t;
#pragma EZ_COMPILER diagnostic pop

#if !defined(INT128)
#define INT128(n) ((int128_t)n)
#endif
#ifndef UINT128_MAX
#define INT128_MAX ((int128_t)(UINT128_MAX >> 1))
#endif
#ifndef INT128_MIN
#define INT128_MIN (-INT128_MAX - 1)
#endif

#endif // EZINT128_HEADER
