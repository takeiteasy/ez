/* ezclipboard.h -- Cross-platform set/get text from/to clipboard
 https://github.com/takeiteasy/
 
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
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef EZCLIPBOARD_HEADER
#define EZCLIPBOARD_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define EZCB_PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define EZCB_PLATFORM_WINDOWS
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define EZCB_PLATFORM_LINUX
#else
#error "Unsupported operating system"
#endif

int ezSetClipboard(const char *str);
const char* ezGetClipboard(void);

#if defined(__cplusplus)
}
#endif
#endif // EZCLIPBOARD_HEADER

#if defined(EZCLIPBOARD_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
#if defined(EZCB_PLATFORM_MAC)
#import <AppKit/AppKit.h>
#elif defined(EZCB_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(EZCB_PLATFORM_LINUX)
#include <gtk/gtk.h>
#endif

int ezSetClipboard(const char *str) {
#if defined(EZCB_PLATFORM_MAC)
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    [pb clearContents];
    [pb setString:@(str)
          forType:NSPasteboardTypeString];
#elif defined(EZCB_PLATFORM_WINDOWS)
    size_t length = strlen(str);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, length + 1);
    if (!hMem)
        return 0;
    char *ptr = (char*)GlobalLock(hMem);
    memcpy(ptr, str, length * sizeof(char));
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    GlobalUnlock(hMem);
    CloseClipboard();
#elif defined(EZCB_PLATFORM_LINUX)
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, str, -1);
#endif
    return 1;
}

const char* ezGetClipboard(void) {
#if defined(EZCB_PLATFORM_MAC)
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    if ([[pb types] containsObject:NSPasteboardTypeString]) {
        NSString *text = [pb stringForType:NSPasteboardTypeString];
        return strdup([text UTF8String]);
    }
    return NULL;
#elif defined(EZCB_PLATFORM_WINDOWS)
    OpenClipboard(NULL);
    char *result = NULL;
    if (!IsClipboardFormatAvailable(CF_TEXT))
        goto BAIL;
    HGLOBAL hMem = GetClipboardData(format);
    if (!hMem)
        goto BAIL;
    char *text = (char*)GlobalLock(hMem);
    result = text ? strdup(text) : NULL;
BAIL:
    GlobalUnlock(hMem);
    CloseClipboard();
    return result;
#elif defined(EZCB_PLATFORM_LINUX)
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gchar *text = gtk_clipboard_wait_for_text(clipboard);
    return text ? strdup((char*)text) : NULL;
#endif
}
#endif
