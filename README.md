# ez

My personal collection of re-usable single-header libraries. They're mostly data collections or cross-platform wrappers. See the [table](#libraries) below for a basic overview of each library.

If you'd like to use any of these libraries, define ```EZ[NAME]_IMPLEMENTATION``` (or just ```EZ_IMPLEMENTATION```) in one of your source files. If you want to use custom malloc/free calls, define ```EZ_MALLOC``` or ```EZ_FREE``` before including. Realloc + calloc are also defined the same way.

## Libraries

| File               | Description                                                   | Additional info                |
| -----------------  | ------------------------------------------------------------- | ------------------------------ |
| **ezarena.h**      | Simple memory arena implementation using mmap+VirtualAlloc    | None |
| **ezclipboard.h**  | Get/set clipboard (text only) on Mac/Windows/Linux (GTK)      | `WIP: Emscripten support` |
| **ezfs.h**         | Common cross-platform file system functions                   | None |
| **ezimage.h**      | Image manipulation, .png importing + exporting                | To disable text-rendering define `EZIMAGE_DISABLE_TEXT` and to disable saving/loading define `EZIMAGE_DISABLE_IO` |
| **ezmap.h**        | Simple key value map + dictionary                             | Some functionality relies on clang+gcc extensions, define `EZMAP_DISABLE_GENERICS` this removes the `ezMap` type |
| **ezrng.h**        | Simple pseudo random number generation                        | None |
| **ezstack.h**      | Simple double linked-list implementation                      | None |
| **ezthreads.h**    | pthreads wrapper for windows + thread pool implementation     | The pthread wrapper for windows is only a partial implementation, not all of the pthread API is covered. Define `EZTHREAD_USE_NATIVE_CALL_ONCE` and `EZTHREAD_USE_NATIVE_CV` to enable native `call_once` and conditional vars support on windows |
| **ezvector.h**     | Stretchy buffer implementation                                | `WIP: Probably could pad the API` |

> [!NOTE]
> Any libraries or code used in these libraries are attributed at the head of each file

## LICENSE
```
My collection of reusable single-header libraries

Copyright (C) 2024  George Watson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. */
```
