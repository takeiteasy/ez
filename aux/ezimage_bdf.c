#define EZIMAGE_IMPLEMENTATION
#include "ezimage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define BDF_READ(PARAM) out->PARAM = atoi(strtok(NULL, " \t\n\r"))
#define HTOI(P) ((P) <= '9' ? (P) - '0' : ((P) <= 'F' ? (P) - 'A' + 10 : (P) - 'a' + 10))
#if defined(_MSC_VER)
#define strcasecmp _strcmpi
#endif

typedef struct {
    unsigned int width;
    char* data;
    int x, y, w, h;
} BdfGlyph;

typedef struct {
    int x, y, w, h;
    unsigned int *encodingTable, sizeOfGlyphs;
    BdfGlyph *glyphs;
} BdfFont;

static void LoadBdfFont(BdfFont *out, const char *path) {
    FILE *fp = fopen(path, "r");
    char *s, *p, buf[BUFSIZ];
    for (;;) {
        if (!fgets(buf, sizeof(buf), fp))
            break;
        if (!(s = strtok(buf, " \t\n\r")))
            break;
        
        if (!strcasecmp((char*)s, "FONTBOUNDINGBOX")) {
            BDF_READ(w);
            BDF_READ(h);
            BDF_READ(x);
            BDF_READ(y);
        } else if (!strcasecmp(s, "CHARS")) {
            BDF_READ(sizeOfGlyphs);
            break;
        }
    }
    out->encodingTable = malloc(out->sizeOfGlyphs * sizeof(unsigned int));
    out->glyphs = malloc(out->sizeOfGlyphs * sizeof(BdfGlyph));
    
    int encoding = 0, width = -1, scanline = -1, i, j, n = 0;
    for (;;) {
        if (!fgets(buf, sizeof(buf), fp))
            break;
        if (!(s = strtok(buf, " \t\n\r")))
            break;
        
        if (!strcasecmp(s, "ENCODING")) {
            encoding = atoi(strtok(NULL, " \t\n\r"));
        } else if (!strcasecmp(s, "DWIDTH")) {
            width = atoi(strtok(NULL, " \t\n\r"));
        } else if (!strcasecmp(s, "BBX")) {
            BDF_READ(glyphs[n].w);
            BDF_READ(glyphs[n].h);
            BDF_READ(glyphs[n].x);
            BDF_READ(glyphs[n].y);
        } else if (!strcasecmp(s, "BITMAP")) {
            if (out->glyphs[n].x < 0) {
                width -= out->glyphs[n].x;
                out->glyphs[n].x = 0;
            }
            if (out->glyphs[n].x + out->glyphs[n].w > width)
                width = out->glyphs[n].x + out->glyphs[n].w;
            
            out->glyphs[n].data = malloc(((out->w + 7) / 8) * out->h * sizeof(unsigned char));
            out->glyphs[n].width = width;
            out->encodingTable[n] = encoding;
            
            scanline = 0;
            memset(out->glyphs[n].data, 0, ((out->w + 7) / 8) * out->h);
        } else if (!strcasecmp(s, "ENDCHAR")) {
            if (out->glyphs[n].x) {
                if (out->glyphs[n].x < 0 || out->glyphs[n].x > 7)
                    continue;
                
                int x, y, c, o;
                for (y = 0; y < out->h; ++y) {
                    o = 0;
                    for (x = 0; x < out->w; x += 8) {
                        c = out->glyphs[n].data[y * ((out->w + 7) / 8) + x / 8];
                        out->glyphs[n].data[y * ((out->w + 7) / 8) + x / 8] = c >> out->glyphs[n].x | o;
                        o = c << (8 - out->glyphs[n].x);
                    }
                }
            }
            
            scanline = -1;
            width = -1;
            ++n;
        } else {
            if (n >= out->sizeOfGlyphs || !out->glyphs[n].data || scanline < 0)
                continue;
            
            p = s;
            j = 0;
            while (*p) {
                i = HTOI(*p);
                ++p;
                if (*p)
                    i = HTOI(*p) | i * 16;
                else {
                    out->glyphs[n].data[j + scanline * ((out->w + 7) / 8)] = i;
                    break;
                }
                
                out->glyphs[n].data[j + scanline * ((out->w + 7) / 8)] = i;
                ++j;
                ++p;
            }
            ++scanline;
        }
    }
    
    fclose(fp);
}

static void DestroyBdfFont(BdfFont *f) {
    for (int i = 0; i < f->sizeOfGlyphs; ++i)
        if (f->glyphs[i].data) {
            free(f->glyphs[i].data);
            f->glyphs[i].data = NULL;
        }
    if (f->glyphs) {
        free(f->glyphs);
        f->glyphs = NULL;
    }
    if (f->encodingTable) {
        free(f->encodingTable);
        f->encodingTable = NULL;
    }
    memset(f, 0, sizeof(sizeof(BdfFont)));
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

static int BdfDrawCharacter(ezImage *image, BdfFont *f, const char *c, int x, int y, int col) {
    int u = -1, i, j, n = 0;
    int l = ctoi(c, &u);
    for (i = 0; i < f->sizeOfGlyphs; ++i)
        if (f->encodingTable[i] == u) {
            n = i;
            break;
        }

    int yoffset = f->h - f->glyphs[n].h + (f->y - f->glyphs[n].y), xx, yy, cc;
    for (yy = 0; yy < f->h; ++yy)
        for (xx = 0; xx < f->w; xx += 8) {
            cc = (yy < yoffset || yy > yoffset + f->glyphs[n].h ? 0 : f->glyphs[n].data[(yy - yoffset) * ((f->w + 7) / 8) + xx / 8]);
            for (i = 128, j = 0; i; i /= 2, ++j)
                if (cc & i)
                    ezImagePSet(image, x + j, y + yy, col);
        }
    
    return l;
}

static void BdfDrawString(ezImage *image, BdfFont *f, const char *str, int x, int y, int col) {
    const char* c = (const char*)str;
    int u = x, v = y;
    while (c && *c != '\0') {
        if (*c == '\n') {
            v += f->h + 2;
            u = x;
            c++;
        } else {
            c += BdfDrawCharacter(image, f, c, u, v, col);
            u += f->w;
        }
    }
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

static void BdfDrawStringFormat(ezImage *image, BdfFont *f, int x, int y, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = _vscprintf(fmt, args) + 1;
    char *str = malloc(sizeof(char) * size);
    vsnprintf(str, size, fmt, args);
    va_end(args);
    BdfDrawString(image, f, str, x, y, col);
    free(str);
}

int main(int argc, const char *argv[]) {
    ezImage *image = ezImageNew(256, 256);
    ezImageFill(image, 0xFF00FF00);
    
    BdfFont font;
    LoadBdfFont(&font, "res/tewi-medium-11.bdf");
    BdfDrawString(image, &font, "This is just a test!", 0, 0, 0xFFFFFFFF);
    
    ezImageSave(image, "test_bdf.png");
    
    ezImageFree(image);
    DestroyBdfFont(&font);
    return 0;
}
