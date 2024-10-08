/* ezfs.h -- https://github.com/takeiteasy/ez
 
 ezfs -- Cross-platform functions to interact with the filesystem
 
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

#ifndef EZFS_HEADER
#define EZFS_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#define EZFS_PLATFORM_POSIX
#if defined(__APPLE__) || defined(__MACH__)
#define EZFS_PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64)
#define EZFS_PLATFORM_WINDOWS
#if !defined(EZFS_PLATFORM_FORCE_POSIX)
#undef EZFS_PLATFORM_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define EZFS_PLATFORM_LINUX
#else
#error "Unsupported operating system"
#endif

#if defined(EZFS_PLATFORM_POSIX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dlfcn.h>
#include <time.h>
#else // Windows
#include <io.h>
#include <windows.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#define getcwd _getcwd
#define chdir _chdir
#endif
#include <assert.h>

#if !defined(EZ_MALLOC)
#define EZ_MALLOC malloc
#endif

#if !defined(MAX_PATH)
#if defined(EZFS_PLATFORM_MAC)
#define MAX_PATH 255
#elif defined(EZFS_PLATFORM_WINDOWS)
#define MAX_PATH 256
#elif defined(EZFS_PLATFORM_LINUX)
#define MAX_PATH 4096
#endif
#endif

#if defined(EZFS_PLATFORM_POSIX)
#define PATH_SEPERATOR '/'
#define PATH_SEPERATOR_STR "/"
#else
#define PATH_SEPERATOR '\\'
#define PATH_SEPERATOR_STR "\\"
#endif

int FileExists(const char *path);
int DirExists(const char *path);
char* FormatString(const char *fmt, ...);
unsigned char* LoadFile(const char *path, size_t *length);
const char* JoinPath(const char *a, const char *b);
const char* UserPath(void);
const char* CurrentDirectory(void);
int SetCurrentDirectory(const char *path);
const char* ResolvePath(const char *path);
const char* FileExt(const char *path);
const char* RemoveExt(const char* path);
const char* FileName(const char *path);
const char* RemoveFileName(const char *path);

#if defined(__cplusplus)
}
#endif
#endif // EZFS_HEADER

#if defined(EZFS_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
int FileExists(const char *path) {
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode);
}

int DirExists(const char *path) {
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

char* FormatString(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args) + 1;
    char *buf = EZ_MALLOC(size);
    vsnprintf(buf, size, fmt, args);
    va_end(args);
    return buf;
}

unsigned char* LoadFile(const char *path, size_t *length) {
    unsigned char *result = NULL;
    size_t sz = -1;
    FILE *fh = fopen(path, "rb");
    if (!fh)
        goto BAIL;
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    result = EZ_MALLOC(sz * sizeof(unsigned char));
    fread(result, sz, 1, fh);
    fclose(fh);
    
BAIL:
    if (length)
        *length = sz;
    return result;
}

static const char *PathSeperator(void) {
    static char c = PATH_SEPERATOR;
    char *seperator = (char*)&c;
    seperator[1] = '\0';
    return seperator;
}

const char* JoinPath(const char *a, const char *b) {
    static char buf[MAX_PATH];
    memset(buf, 0, MAX_PATH * sizeof(char));
    strcat(buf, a);
    if (b) {
        if (a[strlen(a)-1] != PATH_SEPERATOR && b[0] != PATH_SEPERATOR) {
            strcat(buf, PathSeperator());
            strcat(buf, b);
        } else if (a[strlen(a)-1] == PATH_SEPERATOR && b[0] == PATH_SEPERATOR)
            strcat(buf, b + 1);
        else
            strcat(buf, b);
    } else
        strcat(buf, PathSeperator());
    return buf;
}

const char* UserPath(void) {
    const char *result;
    if (!(result = getenv("HOME"))) {
#if defined(EZFS_PLATFORM_POSIX)
        result = getpwuid(getuid())->pw_dir;
#else
        WCHAR profilePath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, profilePath))) {
            static char buffer[MAX_PATH];
            if (wcstombs(buffer, profilePath, sizeof(buffer)) == MAX_PATH)
                buffer[MAX_PATH-1] = '\0';
            return buffer;
        }
#endif
    }
    return result;
}

const char* CurrentDirectory(void) {
    static char buf[MAX_PATH];
    memset(buf, 0, MAX_PATH * sizeof(char));
    getcwd(buf, MAX_PATH);
    size_t length = strlen(buf);
    if (buf[length-1] != PATH_SEPERATOR)
        buf[length] = PATH_SEPERATOR;
    return buf;
}

int SetCurrentDirectory(const char *path) {
    const char *rpath = ResolvePath(path);
    if (DirExists(rpath)) {
        chdir(rpath);
        return 1;
    }
    return 1;
}

static int PathDepth(const char *path) {
    assert(DirExists(path));
    assert(path && path[0] == PATH_SEPERATOR);
    const char *cursor = path + 1;
    int depth = 0;
    char last = '\0';
    while (cursor) {
        last = cursor[0];
        if (last == PATH_SEPERATOR)
            depth++;
        cursor++;
    }
    if (last != PATH_SEPERATOR)
        depth++;
    return depth;
}

static const char* PathParent(const char *path, int parent) {
    static char buf[MAX_PATH];
    memset(buf, 0, MAX_PATH * sizeof(char));
    int depth = PathDepth(path);
    assert(parent < depth);
    const char *cursor = path + 1;
    int length = 0;
    for (int remainder = depth - parent; remainder > 0; remainder--) {
        int waiting = 1;
        while (waiting) {
            if (cursor[0] == PATH_SEPERATOR)
                waiting = 1;
            cursor++;
            length++;
        }
    }
    memcpy(buf, buf, length * sizeof(char));
    return buf;
}

const char* ResolvePath(const char *path) {
    assert(path);
    size_t pathSize = strlen(path);
    switch (path[0]) {
        case '~':
            assert(pathSize > 2);
            return path[1] == PATH_SEPERATOR ? JoinPath(UserPath(), path + 2) : path;
        case '.':
            if (pathSize == 1)
                return CurrentDirectory();
            switch (path[1]) {
                case '.':
                    switch (pathSize) {
                        case 2:
                            return PathParent(path, 1);
                        case 3:
                            if (path[2] == PATH_SEPERATOR)
                                return PathParent(path, 1);
                            break;
                        default:
                            assert(pathSize > 3);
                            assert(path[2] == PATH_SEPERATOR);
                            static char back[3] = { '.', '.', PATH_SEPERATOR };
                            const char *cursor = path + 3;
                            int backtrack = 1;
                            while (cursor) {
                                if (strlen(cursor) < 3 || strncmp(cursor, back, 3))
                                    return JoinPath(PathParent(path, backtrack), cursor);
                                cursor += 3;
                                backtrack++;
                            }
                            return PathParent(path, backtrack);
                    }
                    break;
                case PATH_SEPERATOR:
                    return pathSize == 2 ? CurrentDirectory() : JoinPath(CurrentDirectory(), path + 2);
                default:
                    break;
            }
            break;
    }
    return JoinPath(CurrentDirectory(), path);
}

const char* FileExt(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

const char* RemoveExt(const char* path) {
    char *lastExt = strrchr(path, '.');
    if (lastExt != NULL)
        *lastExt = '\0';
    return lastExt ? path : NULL;
}

const char* FileName(const char *path) {
    unsigned long l = 0;
    char *tmp = strstr(path, PATH_SEPERATOR_STR);
    do {
        l = strlen(tmp) + 1;
        path = &path[strlen(path) - l + 2];
        tmp = strstr(path, PATH_SEPERATOR_STR);
    } while(tmp);
    return path;
}

const char* RemoveFileName(const char *path) {
    static char buf[MAX_PATH];
    memcpy(buf, path, MIN(strlen(path), MAX_PATH));
    char *p = strrchr(buf, '/');
    if (p)
        *p = '\0';
    return buf;
}
#endif
