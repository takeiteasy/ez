# ez 

Single header libraries mostly aimed towards game development. Recently I found myself reusing several pieces of code and often losing track of which version was the latest. So I've compiled them into single-header format to keep track easier.

Simple include the header and define ```EZ[NAME]_IMPLEMENTATION``` (or just ```EZ_IMPLEMENTATION```) in one of your source files. There are no dependencies outside of the standard library unless otherwise stated.


## Libraries

| File              | Description                                                   | Define                         |
| ----------------- | ------------------------------------------------------------- | ------------------------------ |
| **ezcontainer.h** | Simple immutable container format to save/load data from disk | **EZCONTAINER_IMPLEMENTATION** |
| **ezecs.h**§      | Simple entity component system                                | **EZECS_IMPLEMENTATION**       |
| **ezfs.h**§       | Common cross-platform file system functions                   | **EZFS_IMPLEMENTATION**        |
| **ezimage.h**     | Image manipulation, .png importing + exporting                | **EZIMAGE_IMPLEMENTATION**     |
| **ezmath.h**\*    | Common math functions + types                                 | **EZMATH_IMPLEMENTATION**      |
| **ezobjc.h**†     | Wrapper for ObjC runtime                                      | N/A                            |
| **ezrng.h**       | Simple pseudo random number generation                        | **EZRNG_IMPLEMENTATION**       |
| **ezstack.h**§    | Simple double linked-list implementation                      | **EZSTACK_IMPLEMENTATION**     |
| **ezvector.h**‡   | Stretchy buffer implementation                                | **EZVECTOR_IMPLEMENTATION**    |

* **\*** Relies on clang+gcc extensions, define -fenable-matrix when building
* **†** Requires linking to external dependencies
* **‡** Some functionality relies on clang+gcc
* **§** Work in progress, expect massive changes

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
