#define EZIMAGE_IMPLEMENTATION
#include "ezimage.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct {
    unsigned char *data;
    size_t sizeOfData;
    stbtt_fontinfo font;
} TtfFont;

static unsigned char* LoadFileMemory(const char *path, size_t *sizeOfFile) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    unsigned char *data = malloc(size + 1);
    fread(data, size, 1, fp);
    fclose(fp);
    
    data[size] = '\0';
    *sizeOfFile = size;
    return data;
}

void LoadTtfFont(TtfFont *out, const char *path) {
    out->data = LoadFileMemory(path, &out->sizeOfData);
    stbtt_InitFont(&out->font, out->data, stbtt_GetFontOffsetForIndex(out->data, 0));
}

void DestroyTtfFont(TtfFont *font) {
    if (font->data)
        free(font->data);
}

static int ctoi(const char* c, int* out) {
    int u = *c, l = 1;
    if ((u & 0xC0) == 0xC0) {
        int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
        if (a < 6 || !(u & 0x02)) {
            u = ((u << (a + 1)) & 0xFF) >> (a + 1);
            for (int b = 1; b < a; ++b)
                u = (u << 6) | (c[l++] & 0x3F);
        }
    }
    *out = u;
    return l;
}

int TtfDrawCharacter(ezImage *image, TtfFont *font, const char *c, int x, int y, int col) {
    return 0;
}

void TtfDrawString(ezImage *image, TtfFont *font, const char *str, int x, int y, int col) {
    
}

// Taken from: https://stackoverflow.com/a/4785411
//#if !defined(_WIN32) && !defined(_WIN64)
//static int _vscprintf(const char *format, va_list pargs) {
//    va_list argcopy;
//    va_copy(argcopy, pargs);
//    int retval = vsnprintf(NULL, 0, format, argcopy);
//    va_end(argcopy);
//    return retval;
//}
//#endif

void TtfDrawStringFormat(ezImage *image, TtfFont *font, int x, int y, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = _vscprintf(fmt, args) + 1;
    char *str = malloc(sizeof(char) * size);
    vsnprintf(str, size, fmt, args);
    va_end(args);
    TtfDrawString(image, font, str, x, y, col);
    free(str);
}

int main(int argc, const char *argv[]) {
    ezImage *image = ezImageNew(256, 256);
    ezImageFill(image, 0xFFFF0000);
    TtfFont font;
    LoadTtfFont(&font, "res/ComicMono.ttf");
    ezImageSave(image, "test_ttf.png");
    ezImageFree(image);
    DestroyTtfFont(&font);
    return 0;
}
