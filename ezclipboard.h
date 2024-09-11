/* ezclipboard.h -- https://github.com/takeiteasy/ez
 
 ezclipboard -- Cross-platform clipboard wrapper
 
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

#ifndef EZCLIPBOARD_HEADER
#define EZCLIPBOARD_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#if defined(__APPLE__) || defined(__MACH__)
#define EZCB_PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64)
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
