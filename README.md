# ez

Single header libraries mostly aimed towards game development. I have had trouple keeping track of various bits of code, so I decided to consolidate them here. I add to them and modify them as I work on other stuff.

These are for my personal use, however if you'd like to use any of these libraries, define ```EZ[NAME]_IMPLEMENTATION``` (or just ```EZ_IMPLEMENTATION```) in one of your source files. If you want to use custom malloc/free calls, define ```EZ_MALLOC``` or ```EZ_FREE``` before including. Realloc + calloc are also defined the same way.

## Libraries

| File               | Description                                                   | Define                         |
| -----------------  | ------------------------------------------------------------- | ------------------------------ |
| **ezarena.h**      | Simple memory arena implementation using mmap+VirtualAlloc    | **EZARENA_IMPLEMENTATION**     |
| **ezclipboard.h**† | Get/set clipboard (text only) on Mac/Windows/Linux (GTK)      | **EZCLIPBOARD_IMPLEMENTATION** |
| **ezecs.h**§      | Simple entity component system                                | **EZECS_IMPLEMENTATION**       |
| **ezfs.h**†        | Common cross-platform file system functions                   | **EZFS_IMPLEMENTATION**        |
| **ezhttp.h**†      | Simple embedded HTTP server                                   | **EZHTTP_IMPLEMENTATION**      |
| **ezimage.h**      | Image manipulation, .png importing + exporting                | **EZIMAGE_IMPLEMENTATION**     |
| **ezmap.h**‡       | Simple key value map + dictionary                             | **EZMAP_IMPLEMENTATION**       |
| **ezmath.h**‡      | Common math functions + types                                 | **EZMATH_IMPLEMENTATION**      |
| **ezrng.h**†       | Simple pseudo random number generation                        | **EZRNG_IMPLEMENTATION**       |
| **ezstack.h**      | Simple double linked-list implementation                      | **EZSTACK_IMPLEMENTATION**     |
| **ezthreads.h**    | pthreads wrapper for windows + thread pool implementation     | **EZTHREADS_IMPLEMENTATION**   |
| **ezvector.h**‡    | Stretchy buffer implementation                                | **EZVECTOR_IMPLEMENTATION**    |

* **‡** Some functionality relies on clang+gcc extensions
* **†** Work in progress, expect massive changes

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
