//
//  ezmemory.h
//  ez
//
//  Created by George Watson on 19/12/2023.
//
// This exists to copy and paste

#ifndef EZMEMORY_HEADER
#define EZMEMORY_HEADER

#if !defined(EZ_MALLOC)
#define EZ_MALLOC malloc
#endif
#if !defined(EZ_CALLOC)
#define EZ_MALLOC calloc
#endif
#if !defined(EZ_REALLOC)
#define EZ_MALLOC realloc
#endif
#if !defined(EZ_FREE)
#define EZ_MALLOC free
#endif

#endif // EZMEMORY_HEADER
