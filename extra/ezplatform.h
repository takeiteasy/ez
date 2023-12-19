//
//  ezplatform.h
//  ez
//
//  Created by George Watson on 19/12/2023.
//
// This exists to copy and paste

#ifndef EZPLATFORM_HEADER
#define EZPLATFORM_HEADER

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

#endif // EZPLATFORM_HEADER
