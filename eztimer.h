/* eztimer.h - Cross-platform high-performace timer (WIP)
   https://github.com/takeiteasy/
  
 The MIT License (MIT)
 
 Copyright (c) 2023 George Watson
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef EZTIMER_HEADER
#define EZTIMER_HEADER

#define EZ_PLATFORM_POSIX
#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define EZ_PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define EZ_PLATFORM_WINDOWS
#if !defined(FS_PLATFORM_FORCE_POSIX)
#undef EZ_PLATFORM_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define EZ_PLATFORM_LINUX
#else
#define EZ_PLATFORM_UNKNOWN
#endif

// Might work, give it a go anyway
#if defined(EZ_PLATFORM_UNKNOWN)
#define EZ_PLATFORM_LINUX
#endif

#if defined(EZ_PLATFORM_MAC)
#include <mach/mach_time.h>
#elif defined(EZ_PLATFORM_WINDOWS)
#include <windows.h>
#else
#include <time.h>
#endif
#include <stdint.h>

#if defined(__clang__)
#define EZ_COMPILER clang
#elif defined(__GNUC__) || defined(__GNUG__)
#define EZ_COMPILER GCC
#endif

#if defined(EZ_COMPILER) && defined(__SIZEOF__INT128__)
#define EZ_128_AVAILABLE 1
#pragma EZ_COMPILER diagnostic push
#pragma EZ_COMPILER diagnostic ignored "-Wpragmas"
#pragma EZ_COMPILER diagnostic ignored "-Wpedantic"
#pragma EZ_COMPILER diagnostic ignored "-pedantic"
typedef unsigned __int128 uint128_t;
#pragma EZ_COMPILER diagnostic pop

#define UINT128_(n) ((uint128_t)n)
#define UINT128_MAX (~UINT128_C(0))

#pragma EZ_COMPILER diagnostic push
#pragma EZ_COMPILER diagnostic ignored "-Wpragmas"
#pragma EZ_COMPILER diagnostic ignored "-Wpedantic"
#pragma EZ_COMPILER diagnostic ignored "-pedantic"
typedef __int128 int128_t;
#pragma EZ_COMPILER diagnostic pop

#define INT128(n) ((int128_t)n)
#ifndef UINT128_MAX
#define INT128_MAX ((int128_t)(UINT128_MAX >> 1))
#endif
#ifndef INT128_MIN
#define INT128_MIN (-INT128_MAX - 1)
#endif
#else
#define EZ_128_AVAILABLE 0
#endif

#if EZ_128_AVAILABLE
typedef uint128_t ezTimer;
#else
typedef struct ezTimer {
    uint64_t hi;
    uint64_t lo;
} ezTimer;
#endif

ezTimer ezTimerNow(void);
uint64_t ezTimerElapsed(ezTimer timer);

#endif // EZTIMER_HEADER

#if defined(EZTIMER_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
ezTimer ezTimerNow(void) {
#if defined(EZ_PLATFORM_MAC)
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    uint64_t lo = info.denom * 1000L / info.numer;
    uint64_t hi = mach_absolute_time();
#elif defined(EZ_PLATFORM_WINDOWS)
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    uint64_t lo = !QueryPerformanceFrequency(&frequency) ? 1000L : frequency.QuadPart / 1000L;
    uint64_t hi = !QueryPerformanceCounter(&counter) ? timeGetTime() : counter.QuadPart;
#else // EZ_PLATFORM_LINUX
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t ticks = now.tv_sec * 1000L;
    ticks += now.tv_nsec / 1000000L;
    uint64_t lo = 1000000L;
    uint64_t hi = ticks;
#endif
#if EZ_128_AVAILABLE
    return ((uint128_t)hi << 64) | lo;
#else
    return (ezTimer){.hi = hi, .lo = lo};
#endif
}

uint64_t ezTimerElapsed(ezTimer timer) {
    ezTimer now = ezTimer();
#if EZ_128_AVAILABLE
    return (now >> 64) - (timer >> 64);
#else
    return now.hi - timer.hi;
#endif
}
#endif
