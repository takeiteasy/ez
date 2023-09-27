/* ezobjc.h -- Wrapper for ObjC runtime
 https://github.com/takeiteasy
 
 Based off: https://github.com/jimon/osx_app_in_plain_c
 
 --------------------------------------------------------------------

 Objective-C:
    [NSApplication sharedApplication];

 C:
    Class NSApplicationClass = objc_getClass("NSApplication");
    SEL sharedApplicationSel = sel_registerName("sharedApplication");
    ((id (*)(Class, SEL))objc_msgSend)(NSApplicationClass, sharedApplicationSel);

 Wrapper:
    ObjC(id)(class(NSApplication), sel(sharedApplication))
 
 --------------------------------------------------------------------
 
 Objective-C:
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

 C:
    SEL setActivationPolicySel = sel_registerName("setActivationPolicy:");
 ((void (*)(id, SEL, NSInteger))objc_msgSend)(NSApp, setActivationPolicySel, 0);

 Wrapper:
    ObjC(void, NSInteger)(NSApp, sel(setActivationPolicy:), 0)
          ^       ^         ^           ^                   ^
     ReturnType ArgTypes   Class     Selectors          Arguemnts
 
 --------------------------------------------------------------------
 
 The MIT License (MIT)
 Copyright (c) 2022 George Watson
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
 
 -------------------------------------------------------------------- */

#ifndef EZOBJC_HEADER
#define EZOBJC_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>

// maybe this is available somewhere in objc runtime?
#if __LP64__ || (TARGET_OS_EMBEDDED && !TARGET_OS_IPHONE) || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
#define NSIntegerEncoding "q"
#define NSUIntegerEncoding "L"
#else
#define NSIntegerEncoding "i"
#define NSUIntegerEncoding "I"
#endif

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
// this is how they are defined originally
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>

typedef CGPoint NSPoint;
typedef CGSize NSSize;
typedef CGRect NSRect;

extern id NSApp;
extern id const NSDefaultRunLoopMode;
#endif

// ABI is a bit different between platforms
#ifdef __arm64__
#define abi_objc_msgSend_stret objc_msgSend
#else
#define abi_objc_msgSend_stret objc_msgSend_stret
#endif
#ifdef __i386__
#define abi_objc_msgSend_fpret objc_msgSend_fpret
#else
#define abi_objc_msgSend_fpret objc_msgSend
#endif

#define __STRINGIFY(s) #s

#define sel(NAME) sel_registerName(__STRINGIFY(NAME))
#define class(NAME) ((id)objc_getClass(__STRINGIFY(NAME)))
#define protocol(NAME) objc_getProtocol(__STRINGIFY(NAME))

#define ObjC_Super(RET, ...) ((RET(*)(struct objc_super*, SEL, ##__VA_ARGS__))objc_msgSendSuper)
#define ObjC_Set(OBJ, VAR, VAL) object_setInstanceVariable(OBJ, __STRINGIFY(VAR), (void*)VAL)
#define ObjC_Get(OBJ, VAR, OUT) object_getInstanceVariable(self, __STRINGIFY(VAR), (void**)&OUT)
#define ObjC_SelfSet(VAR, VAL) ObjC_Set(self, VAR, VAL)
#define ObjC_SelfGet(VAR, OUT) ObjC_Get(self, VAR, OUT)
#define ObjC(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))objc_msgSend)
// ObjC functions that return regular structs (e.g. NSRect) must use this
#define ObjC_Struct(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))abi_objc_msgSend_stret)

#define ObjC_Class(NAME, SUPER) \
    objc_allocateClassPair((Class)objc_getClass(__STRINGIFY(SUPER)), __STRINGIFY(NAME), 0)
#define ObjC_AddMethod(CLASS, SEL, IMPL, SIGNATURE)                  \
    if (!class_addMethod(CLASS, sel(SEL), (IMP)(IMPL), (SIGNATURE))) \
        assert(false)
#define ObjC_AddIVar(CLASS, NAME, SIZE, SIGNATURE)                                   \
    if (!class_addIvar(CLASS, __STRINGIFY(NAME), SIZE, rint(log2(SIZE)), SIGNATURE)) \
        assert(false)
#define ObjC_AddProtocol(CLASS, PROTOCOL)                           \
    if (!class_addProtocol(CLASS, protocol(__STRINGIFY(PROTOCOL)))) \
        assert(false);
#define ObjC_SubClass(NAME) objc_registerClassPair(NAME)

#if defined(__OBJC__) && __has_feature(objc_arc) && !defined(OBJC_NO_ARC)
#define OBJC_ARC_AVAILABLE
#endif

#if defined(OBJC_ARC_AVAILABLE)
#define ObjC_Autorelease(CLASS)
#define AutoreleasePool(...) \
    do                       \
    {                        \
        @autoreleasepool     \
        {                    \
            __VA_ARGS__      \
        }                    \
    } while (0)
#else
#define ObjC_Autorelease(CLASS) ObjC(void)(class(CLASS), sel(autorelease))
#define AutoreleasePool(...)                                \
    do                                                      \
    {                                                       \
        id __OBJC_POOL = ObjC_Initalize(NSAutoreleasePool); \
        __VA_ARGS__                                         \
        ObjC(void)(__OBJC_POOL, sel(drain));                \
    } while (0)
#endif
#define ObjC_Alloc(CLASS) ObjC(id)(class(CLASS), sel(alloc))
#define ObjC_Init(CLASS) ObjC(id)(CLASS, sel(init))
#define ObjC_Initalize(CLASS) ObjC(id)(ObjC_Alloc(CLASS), sel(init))
#define ObjC_Release(CLASS) ObjC(void)(CLASS, sel(release))

#if defined(__cplusplus)
}
#endif
#endif // EZOBJC_HEADER
