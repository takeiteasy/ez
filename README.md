# ez

My personal collection of re-usable single-header libraries. They're mostly data collections or cross-platform wrappers. See the [table](#libraries) below for a basic overview of each library.

If you'd like to use any of these libraries, define ```EZ[NAME]_IMPLEMENTATION``` (or just ```EZ_IMPLEMENTATION```) in one of your source files. If you want to use custom malloc/free calls, define ```EZ_MALLOC``` or ```EZ_FREE``` before including. Realloc + calloc are also defined the same way.

## Libraries

| File               | Description                                                   | Additional info                |
| -----------------  | ------------------------------------------------------------- | ------------------------------ |
| **ezarena.h**      | Simple memory arena implementation using mmap+VirtualAlloc    | None                           |
| **ezclipboard.h**  | Get/set clipboard (text only) on Mac/Windows/Linux (GTK)      | `WIP: Emscripten support`      |
| **ezfs.h**         | Common cross-platform file system functions                   | None                           |
| **ezimage.h**      | Image manipulation, .png importing + exporting                | To disable text-rendering define `EZIMAGE_DISABLE_TEXT` and to disable saving/loading define `EZIMAGE_DISABLE_IO` |
| **ezmap.h**        | Simple key value map + dictionary                             | Some functionality relies on clang+gcc extensions, define `EZMAP_DISABLE_GENERICS` this removes the `ezMap` type |
| **eznet.h**        | Simple cross-platform TCP+UDP network library                 | `WIP: Not finished do not use` |
| **ezpoll.h**       | Very basic wrapper over epoll, kqueue and iocp                | `WIP: Not finished do not use` | 
| **ezrng.h**        | Simple pseudo random number generation                        | None                           |
| **ezstack.h**      | Simple double linked-list implementation                      | None                           |
| **ezthreads.h**    | pthreads wrapper for windows + thread pool implementation     | The pthread wrapper for windows is only a partial implementation, not all of the pthread API is covered. Define `EZTHREAD_USE_NATIVE_CALL_ONCE` and `EZTHREAD_USE_NATIVE_CV` to enable native `call_once` and conditional vars support on windows |
| **ezvector.h**     | Stretchy buffer implementation                                | Based off stretchy_buffer.h by Sean Barrett (nothings/sbt) |


## LICENSE
```
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
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
œ
