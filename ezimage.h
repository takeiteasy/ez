/* ezimage.h - Image manipulation, importing + exporting
   https://github.com/takeiteasy/
  
 Acknowledgements:
 
  - PNG importing/exporting was adapted from https://github.com/erkkah/tigr (UNLICENSED)
    All the hard work was done by erkkah, I only modified it to fit with this library
  - 8x8 font was adapted from https://github.com/dhepper/font8x8/ (Public Domain)
 
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

#ifndef EZIMAGE_HEADER
#define EZIMAGE_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <setjmp.h>
#include <errno.h>

int RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
int RGB(unsigned char r, unsigned char g, unsigned char b);
int RGBA1(unsigned char c, unsigned char a);
int RGB1(unsigned char c);
unsigned char Rgba(int c);
unsigned char rGba(int c);
unsigned char rgBa(int c);
unsigned char rgbA(int c);
int rGBA(int c, unsigned char r);
int RgBA(int c, unsigned char g);
int RGbA(int c, unsigned char b);
int RGBa(int c, unsigned char a);

typedef struct ezImage {
    int *buf, w, h;
} ezImage;

ezImage* ezImageNew(unsigned int w, unsigned int h);
void ezImageFree(ezImage *img);

void ezImageFill(ezImage *img, int col);
void ezImageFlood(ezImage *img, int x, int y, int col);
void ezImagePSet(ezImage *img, int x, int y, int col);
int ezImagePGet(ezImage *img, int x, int y);
int ezImagePaste(ezImage *dst, ezImage *src, int x, int y);
int ezImagePasteClipped(ezImage *dst, ezImage *src, int x, int y, int rx, int ry, int rw, int rh);
ezImage*  ezImageDupe(ezImage *src);
void ezImagePassThru(ezImage *img, int(*fn)(int x, int y, int col));
ezImage*  ezImageResize(ezImage *src, int nw, int nh);
ezImage*  ezImageRotate(ezImage *src, float angle);
void ezImageDrawLine(ezImage *img, int x0, int y0, int x1, int y1, int col);
void ezImageDrawCircle(ezImage *img, int xc, int yc, int r, int col, int fill);
void ezImageDrawRectangle(ezImage *img, int x, int y, int w, int h, int col, int fill);
void ezImageDrawTriangle(ezImage *img, int x0, int y0, int x1, int y1, int x2, int y2, int col, int fill);

void ezImageDrawCharacter(ezImage *img, char c, int x, int y, int col);
void ezImageDrawString(ezImage *img, const char *str, int x, int y, int col);
void ezImageDrawStringFormat(ezImage *img, int x, int y, int col, const char *fmt, ...);

ezImage*  ezImageLoadFromPath(const char *path);
ezImage*  ezImageLoadFromMemory(const void *data, size_t length);
int ezImageSave(ezImage *img, const char *path);

#if defined(__cplusplus)
}
#endif
#endif // EZIMAGE_HEADER

#if defined(EZIMAGE_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
int RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
}

int RGB(unsigned char r, unsigned char g, unsigned char b) {
    return RGBA(r, g, b, 255);
}

int RGBA1(unsigned char c, unsigned char a) {
    return RGBA(c, c, c, a);
}

int RGB1(unsigned char c) {
    return RGB(c, c, c);
}

unsigned char Rgba(int c) {
    return (unsigned char)((c >> 16) & 0xFF);
}

unsigned char rGba(int c) {
    return (unsigned char)((c >>  8) & 0xFF);
}

unsigned char rgBa(int c) {
    return (unsigned char)(c & 0xFF);
}

unsigned char rgbA(int c) {
    return (unsigned char)((c >> 24) & 0xFF);
}

int rGBA(int c, unsigned char r) {
    return (c & ~0x00FF0000) | (r << 16);
}

int RgBA(int c, unsigned char g) {
    return (c & ~0x0000FF00) | (g << 8);
}

int RGbA(int c, unsigned char b) {
    return (c & ~0x000000FF) | b;
}

int RGBa(int c, unsigned char a) {
    return (c & ~0x00FF0000) | (a << 24);
}

ezImage* ezImageNew(unsigned int w, unsigned int h) {
    ezImage *result = malloc(sizeof(ezImage));
    result->w = w;
    result->h = h;
    result->buf = malloc(w * h * sizeof(int));
    return result;
}

void ezImageFree(ezImage *img) {
    if (img) {
        if (img->buf)
            free(img->buf);
        free(img);
    }
}

void ezImageFill(ezImage *img, int col) {
    for (int i = 0; i < img->w * img->h; ++i)
        img->buf[i] = col;
}

static inline void flood_fn(ezImage *img, int x, int y, int new, int old) {
    if (new == old || ezImagePGet(img, x, y) != old)
        return;
    
    int x1 = x;
    while (x1 < img->w && ezImagePGet(img, x1, y) == old) {
        ezImagePSet(img, x1, y, new);
        x1++;
    }
    
    x1 = x - 1;
    while (x1 >= 0 && ezImagePGet(img, x1, y) == old) {
        ezImagePSet(img, x1, y, new);
        x1--;
    }
    
    x1 = x;
    while (x1 < img->w && ezImagePGet(img, x1, y) == new) {
        if(y > 0 && ezImagePGet(img, x1, y - 1) == old)
            flood_fn(img, x1, y - 1, new, old);
        x1++;
    }
    
    x1 = x - 1;
    while(x1 >= 0 && ezImagePGet(img, x1, y) == new) {
        if(y > 0 && ezImagePGet(img, x1, y - 1) == old)
            flood_fn(img, x1, y - 1, new, old);
        x1--;
    }
    
    x1 = x;
    while(x1 < img->w && ezImagePGet(img, x1, y) == new) {
        if(y < img->h - 1 && ezImagePGet(img, x1, y + 1) == old)
            flood_fn(img, x1, y + 1, new, old);
        x1++;
    }
    
    x1 = x - 1;
    while(x1 >= 0 && ezImagePGet(img, x1, y) == new) {
        if(y < img->h - 1 && ezImagePGet(img, x1, y + 1) == old)
            flood_fn(img, x1, y + 1, new, old);
        x1--;
    }
}

void ezImageFlood(ezImage *img, int x, int y, int col) {
    if (x < 0 || y < 0 || x >= img->w || y >= img->h)
        return;
    flood_fn(img, x, y, col, ezImagePGet(img, x, y));
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

static int Blend(int _a, int _b) {
    int a   = rgbA(_a);
    int b   = rgbA(_b);
    return !a ? 0xFF000000 : a >= 255 ? RGBa(a, 255) : RGBA(BLEND(Rgba(_a), Rgba(_b), a, b),
                                                            BLEND(rGba(_a), rGba(_b), a, b),
                                                            BLEND(rgBa(_a), rgBa(_b), a, b),
                                                            a + (b * (255 - a) >> 8));
}

void ezImagePSet(ezImage *img, int x, int y, int col) {
    if (x >= 0 && y >= 0 && x < img->w && y < img->h)
        img->buf[y * img->w + x] = rgbA(col) == 255 ? col : Blend(ezImagePGet(img, x, y), col);
}

int ezImagePGet(ezImage *img, int x, int y) {
    return (x >= 0 && y >= 0 && x < img->w && y < img->h) ? img->buf[y * img->w + x] : 0;
}

int ezImagePaste(ezImage *dst, ezImage *src, int x, int y) {
    int ox, oy, c;
    for (ox = 0; ox < src->w; ++ox) {
        for (oy = 0; oy < src->h; ++oy) {
            if (oy > dst->h)
                break;
            c = ezImagePGet(src, ox, oy);
            ezImagePSet(dst, x + ox, y + oy, c);
        }
        if (ox > dst->w)
            break;
    }
    return 1;
}

int ezImagePasteClipped(ezImage *dst, ezImage *src, int x, int y, int rx, int ry, int rw, int rh) {
    for (int ox = 0; ox < rw; ++ox)
        for (int oy = 0; oy < rh; ++oy)
            ezImagePSet(dst, ox + x, oy + y, ezImagePGet(src, ox + rx, oy + ry));
    return 1;
}

ezImage* ezImageDupe(ezImage *src) {
    ezImage *result = ezImageNew(src->w, src->h);
    memcpy(result->buf, src->buf, src->w * src->h * sizeof(int));
    return result;
}

void ezImagePassThru(ezImage *img, int (*fn)(int x, int y, int col)) {
    int x, y;
    for (x = 0; x < img->w; ++x)
        for (y = 0; y < img->h; ++y)
            img->buf[y * img->w + x] = fn(x, y, ezImagePGet(img, x, y));
}

ezImage* ezImageResize(ezImage *src, int nw, int nh) {
    ezImage *result = ezImageNew(nw, nh);
    int x_ratio = (int)((src->w << 16) / result->w) + 1;
    int y_ratio = (int)((src->h << 16) / result->h) + 1;
    int x2, y2, i, j;
    for (i = 0; i < result->h; ++i) {
        int *t = result->buf + i * result->w;
        y2 = ((i * y_ratio) >> 16);
        int *p = src->buf + y2 * src->w;
        int rat = 0;
        for (j = 0; j < result->w; ++j) {
            x2 = (rat >> 16);
            *t++ = p[x2];
            rat += x_ratio;
        }
    }
    return result;
}

#define __MIN(a, b) (((a) < (b)) ? (a) : (b))
#define __MAX(a, b) (((a) > (b)) ? (a) : (b))
#define __D2R(a) ((a) * M_PI / 180.0)

ezImage* ezImageRotate(ezImage *src, float angle) {
    float theta = __D2R(angle);
    float c = cosf(theta), s = sinf(theta);
    float r[3][2] = {
        { -src->h * s, src->h * c },
        {  src->w * c - src->h * s, src->h * c + src->w * s },
        {  src->w * c, src->w * s }
    };
    
    float mm[2][2] = {{
        __MIN(0, __MIN(r[0][0], __MIN(r[1][0], r[2][0]))),
        __MIN(0, __MIN(r[0][1], __MIN(r[1][1], r[2][1])))
    }, {
        (theta > 1.5708  && theta < 3.14159 ? 0.f : __MAX(r[0][0], __MAX(r[1][0], r[2][0]))),
        (theta > 3.14159 && theta < 4.71239 ? 0.f : __MAX(r[0][1], __MAX(r[1][1], r[2][1])))
    }};
    
    int dw = (int)ceil(fabsf(mm[1][0]) - mm[0][0]);
    int dh = (int)ceil(fabsf(mm[1][1]) - mm[0][1]);
    ezImage *result = ezImageNew(dw, dh);
    
    int x, y, sx, sy;
    for (x = 0; x < dw; ++x)
        for (y = 0; y < dh; ++y) {
            sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
            sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
            if (sx < 0 || sx >= src->w || sy < 0 || sy >= src->h)
                continue;
            ezImagePSet(result, x, y, ezImagePGet(src, sx, sy));
        }
    return result;
}

static inline void vline(ezImage *img, int x, int y0, int y1, int col) {
    if (y1 < y0) {
        y0 += y1;
        y1  = y0 - y1;
        y0 -= y1;
    }
    
    if (x < 0 || x >= img->w || y0 >= img->h)
        return;
    
    if (y0 < 0)
        y0 = 0;
    if (y1 >= img->h)
        y1 = img->h - 1;
    
    for(int y = y0; y <= y1; y++)
        ezImagePSet(img, x, y, col);
}

static inline void hline(ezImage *img, int y, int x0, int x1, int col) {
    if (x1 < x0) {
        x0 += x1;
        x1  = x0 - x1;
        x0 -= x1;
    }
    
    if (y < 0 || y >= img->h || x0 >= img->w)
        return;
    
    if (x0 < 0)
        x0 = 0;
    if (x1 >= img->w)
        x1 = img->w - 1;
    
    for(int x = x0; x <= x1; x++)
        ezImagePSet(img, x, y, col);
}

void ezImageDrawLine(ezImage *img, int x0, int y0, int x1, int y1, int col) {
    if (x0 == x1)
        vline(img, x0, y0, y1, col);
    if (y0 == y1)
        hline(img, y0, x0, x1, col);
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    
    while (ezImagePSet(img, x0, y0, col), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void ezImageDrawCircle(ezImage *img, int xc, int yc, int r, int col, int fill) {
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        ezImagePSet(img, xc - x, yc + y, col);    /*   I. Quadrant */
        ezImagePSet(img, xc - y, yc - x, col);    /*  II. Quadrant */
        ezImagePSet(img, xc + x, yc - y, col);    /* III. Quadrant */
        ezImagePSet(img, xc + y, yc + x, col);    /*  IV. Quadrant */
        
        if (fill) {
            hline(img, yc - y, xc - x, xc + x, col);
            hline(img, yc + y, xc - x, xc + x, col);
        }
        
        r = err;
        if (r <= y)
            err += ++y * 2 + 1; /* e_xy+e_y < 0 */
        if (r > x || err > y)
            err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

void ezImageDrawRectangle(ezImage *img, int x, int y, int w, int h, int col, int fill) {
    if (x < 0) {
        w += x;
        x  = 0;
    }
    if (y < 0) {
        h += y;
        y  = 0;
    }
    
    w += x;
    h += y;
    if (w < 0 || h < 0 || x > img->w || y > img->h)
        return;
    
    if (w > img->w)
        w = img->w;
    if (h > img->h)
        h = img->h;
    
    if (fill) {
        for (; y < h; ++y)
            hline(img, y, x, w, col);
    } else {
        hline(img, y, x, w, col);
        hline(img, h, x, w, col);
        vline(img, x, y, h, col);
        vline(img, w, y, h, col);
    }
}

#define __SWAP(a, b)  \
    do                \
    {                 \
        int temp = a; \
        a = b;        \
        b = temp;     \
    } while (0)

void ezImageDrawTriangle(ezImage *img, int x0, int y0, int x1, int y1, int x2, int y2, int col, int fill) {
    if (y0 ==  y1 && y0 ==  y2)
        return;
    if (fill) {
        if (y0 > y1) {
            __SWAP(x0, x1);
            __SWAP(y0, y1);
        }
        if (y0 > y2) {
            __SWAP(x0, x2);
            __SWAP(y0, y2);
        }
        if (y1 > y2) {
            __SWAP(x1, x2);
            __SWAP(y1, y2);
        }
        
        int total_height = y2 - y0, i, j;
        for (i = 0; i < total_height; ++i) {
            int second_half = i > y1 - y0 || y1 == y0;
            int segment_height = second_half ? y2 - y1 : y1 - y0;
            float alpha = (float)i / total_height;
            float beta  = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
            int ax = x0 + (x2 - x0) * alpha;
            int ay = y0 + (y2 - y0) * alpha;
            int bx = second_half ? x1 + (x2 - x1) : x0 + (x1 - x0) * beta;
            int by = second_half ? y1 + (y2 - y1) : y0 + (y1 - y0) * beta;
            if (ax > bx) {
                __SWAP(ax, bx);
                __SWAP(ay, by);
            }
            for (j = ax; j <= bx; ++j)
                ezImagePSet(img, j, y0 + i, col);
        }
    } else {
        ezImageDrawLine(img, x0, y0, x1, y1, col);
        ezImageDrawLine(img, x1, y1, x2, y2, col);
        ezImageDrawLine(img, x2, y2, x0, y0, col);
    }
}

#if !defined(_WIN32) && !defined(_WIN64)
// Taken from: https://stackoverflow.com/a/4785411
static int _vscprintf(const char *format, va_list pargs) {
    va_list argcopy;
    va_copy(argcopy, pargs);
    int retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif

static char font8x8_basic[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // U+0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // U+0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // U+0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // U+0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // U+0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // U+0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // U+0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // U+002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // U+002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // U+002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // U+002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // U+0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // U+0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // U+0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // U+0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // U+0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // U+0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // U+0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // U+0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // U+0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // U+0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // U+003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // U+003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // U+003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // U+003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // U+0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // U+0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // U+0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // U+0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // U+0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // U+0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // U+0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // U+0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // U+0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // U+004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // U+004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // U+004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // U+004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // U+004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // U+004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // U+0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // U+0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // U+0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // U+0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U+0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // U+0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // U+0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // U+0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // U+005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // U+005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // U+005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // U+005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // U+005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // U+005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // U+0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // U+0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // U+0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // U+0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // U+0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // U+0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // U+0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // U+006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // U+006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // U+006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // U+006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // U+006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // U+0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // U+0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // U+0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // U+0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // U+0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // U+0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // U+0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // U+0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // U+007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // U+007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // U+007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // U+007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // U+007F
};

void ezImageDrawCharacter(ezImage *img, char c, int x, int y, int col) {
    char *bitmap = font8x8_basic[(int)c];
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            ezImagePSet(img, x + i, y + j, bitmap[j] & 1 << i ? col : 0xFF000000);
}

void ezImageDrawString(ezImage *img, const char *str, int x, int y, int col) {
    int nx = x, ny = y;
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == '\n') {
            ny += 10;
            nx = x;
        } else {
            ezImageDrawCharacter(img, str[i], nx, ny, col);
            nx += 8;
        }
    }
}

void ezImageDrawStringFormat(ezImage *img, int x, int y, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = _vscprintf(fmt, args) + 1;
    char *str = malloc(sizeof(char) * size);
    vsnprintf(str, size, fmt, args);
    va_end(args);
    ezImageDrawString(img, str, x, y, col);
    free(str);
}

typedef struct {
    const unsigned char *p, *end;
} PNG;

static unsigned get32(const unsigned char *v) {
    return (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
}

static const unsigned char* find(PNG *png, const char *chunk, unsigned minlen) {
    const unsigned char *start;
    while (png->p < png->end) {
        unsigned len = get32(png->p + 0);
        start = png->p;
        png->p += len + 12;
        if (memcmp(start + 4, chunk, 4) == 0 && len >= minlen && png->p <= png->end)
            return start + 8;
    }
    return NULL;
}

static unsigned char paeth(unsigned char a, unsigned char b, unsigned char c) {
    int p = a + b - c;
    int pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
    return (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
}

static int rowBytes(int w, int bipp) {
    int rowBits = w * bipp;
    return rowBits / 8 + ((rowBits % 8) ? 1 : 0);
}

static int unfilter(int w, int h, int bipp, unsigned char *raw) {
    int len = rowBytes(w, bipp);
    int bpp = rowBytes(1, bipp);
    int x, y;
    unsigned char *first = (unsigned char*)malloc(len + 1);
    memset(first, 0, len + 1);
    unsigned char *prev = first;
    for (y = 0; y < h; y++, prev = raw, raw += len) {
#define LOOP(A, B)            \
    for (x = 0; x < bpp; x++) \
        raw[x] += A;          \
    for (; x < len; x++)      \
        raw[x] += B;          \
    break
        switch (*raw++) {
            case 0:
                break;
            case 1:
                LOOP(0, raw[x - bpp]);
            case 2:
                LOOP(prev[x], prev[x]);
            case 3:
                LOOP(prev[x] / 2, (raw[x - bpp] + prev[x]) / 2);
            case 4:
                LOOP(prev[x], paeth(raw[x - bpp], prev[x], prev[x - bpp]));
            default:
                return 0;
        }
#undef LOOP
    }
    free(first);
    return 1;
}

static void convert(int bypp, int w, int h, const unsigned char *src, int *dest, const unsigned char *trns) {
    int x, y;
    for (y = 0; y < h; y++) {
        src++;  // skip filter byte
        for (x = 0; x < w; x++, src += bypp) {
            switch (bypp) {
                case 1: {
                    unsigned char c = src[0];
                    if (trns && c == *trns) {
                        *dest++ = RGBA1(c, 0);
                        break;
                    } else {
                        *dest++ = RGB1(c);
                        break;
                    }
                }
                case 2:
                    *dest++ = RGBA(src[0], src[0], src[0], src[1]);
                    break;
                case 3: {
                    unsigned char r = src[0];
                    unsigned char g = src[1];
                    unsigned char b = src[2];
                    if (trns && trns[1] == r && trns[3] == g && trns[5] == b) {
                        *dest++ = RGBA(r, g, b, 0);
                        break;
                    } else {
                        *dest++ = RGB(r, g, b);
                        break;
                    }
                }
                case 4:
                    *dest++ = RGBA(src[0], src[1], src[2], src[3]);
                    break;
            }
        }
    }
}

static void depalette(int w, int h, unsigned char *src, int *dest, int bipp, const unsigned char *plte, const unsigned char *trns, int trnsSize) {
    int x, y, c;
    unsigned char alpha;
    int mask, len;

    switch (bipp) {
        case 4:
            mask = 15;
            len = 1;
            break;
        case 2:
            mask = 3;
            len = 3;
            break;
        case 1:
            mask = 1;
            len = 7;
    }

    for (y = 0; y < h; y++) {
        src++;  // skip filter byte
        for (x = 0; x < w; x++) {
            if (bipp == 8) {
                c = *src++;
            } else {
                int pos = x & len;
                c = (src[0] >> ((len - pos) * bipp)) & mask;
                if (pos == len) {
                    src++;
                }
            }
            alpha = 255;
            if (c < trnsSize) {
                alpha = trns[c];
            }
            *dest++ = RGBA(plte[c * 3 + 0], plte[c * 3 + 1], plte[c * 3 + 2], alpha);
        }
    }
}

static int outsize(ezImage *img, int bipp) {
    return (rowBytes(img->w, bipp) + 1) * img->h;
}

#define PNG_FAIL()      \
    {                   \
        errno = EINVAL; \
        goto err;       \
    }
#define PNG_CHECK(X) \
    if (!(X))        \
        PNG_FAIL()

typedef struct {
    unsigned bits, count;
    const unsigned char *in, *inend;
    unsigned char *out, *outend;
    jmp_buf jmp;
    unsigned litcodes[288], distcodes[32], lencodes[19];
    int tlit, tdist, tlen;
} State;

#define INFLATE_FAIL() longjmp(s->jmp, 1)
#define INFLATE_CHECK(X) \
    if (!(X))            \
        INFLATE_FAIL()

// Built-in DEFLATE standard tables.
static char order[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
static char lenBits[29 + 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0 };
static int lenBase[29 + 2] = { 3,  4,  5,  6,  7,  8,  9,  10,  11,  13,  15,  17,  19,  23, 27, 31,
                               35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0,  0 };
static char distBits[30 + 2] = { 0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,  6, 6,
                                 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0 };
static int distBase[30 + 2] = {
    1,   2,   3,   4,   5,   7,    9,    13,   17,   25,   33,   49,   65,    97,    129,
    193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

static const unsigned char reverseTable[256] = {
#define R2(n) n, n + 128, n + 64, n + 192
#define R4(n) R2(n), R2(n + 32), R2(n + 16), R2(n + 48)
#define R6(n) R4(n), R4(n + 8), R4(n + 4), R4(n + 12)
    R6(0), R6(2), R6(1), R6(3)
};

static unsigned rev16(unsigned n) {
    return (reverseTable[n & 0xff] << 8) | reverseTable[(n >> 8) & 0xff];
}

static int bits(State *s, int n) {
    int v = s->bits & ((1 << n) - 1);
    s->bits >>= n;
    s->count -= n;
    while (s->count < 16) {
        INFLATE_CHECK(s->in != s->inend);
        s->bits |= (*s->in++) << s->count;
        s->count += 8;
    }
    return v;
}

static unsigned char *emit(State *s, int len) {
    s->out += len;
    INFLATE_CHECK(s->out <= s->outend);
    return s->out - len;
}

static void copy(State *s, const unsigned char *src, int len) {
    unsigned char *dest = emit(s, len);
    while (len--)
        *dest++ = *src++;
}

static int build(State *s, unsigned *tree, unsigned char *lens, int symcount) {
    int n, codes[16], first[16], counts[16] = { 0 };

    // Frequency count.
    for (n = 0; n < symcount; n++)
        counts[lens[n]]++;

    // Distribute codes.
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; n++) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }
    INFLATE_CHECK(first[15] + counts[15] <= symcount);

    // Insert keys into the tree for each symbol.
    for (n = 0; n < symcount; n++) {
        int len = lens[n];
        if (len != 0) {
            int code = codes[len]++, slot = first[len]++;
            tree[slot] = (code << (32 - len)) | (n << 4) | len;
        }
    }

    return first[15];
}

static int decode(State *s, unsigned tree[], int max) {
    // Find the next prefix code.
    unsigned lo = 0, hi = max, key;
    unsigned search = (rev16(s->bits) << 16) | 0xffff;
    while (lo < hi) {
        unsigned guess = (lo + hi) / 2;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    // Pull out the key and check it.
    key = tree[lo - 1];
    INFLATE_CHECK(((search ^ key) >> (32 - (key & 0xf))) == 0);

    bits(s, key & 0xf);
    return (key >> 4) & 0xfff;
}

static void run(State *s, int sym) {
    int length = bits(s, lenBits[sym]) + lenBase[sym];
    int dsym = decode(s, s->distcodes, s->tdist);
    int offs = bits(s, distBits[dsym]) + distBase[dsym];
    copy(s, s->out - offs, length);
}

static void block(State *s) {
    for (;;) {
        int sym = decode(s, s->litcodes, s->tlit);
        if (sym < 256)
            *emit(s, 1) = (unsigned char)sym;
        else if (sym > 256)
            run(s, sym - 257);
        else
            break;
    }
}

static void stored(State *s) {
    // Uncompressed data block.
    int len;
    bits(s, s->count & 7);
    len = bits(s, 16);
    INFLATE_CHECK(((len ^ s->bits) & 0xffff) == 0xffff);
    INFLATE_CHECK(s->in + len <= s->inend);

    copy(s, s->in, len);
    s->in += len;
    bits(s, 16);
}

static void fixed(State *s) {
    // Fixed set of Huffman codes.
    int n;
    unsigned char lens[288 + 32];
    for (n = 0; n <= 143; n++)
        lens[n] = 8;
    for (n = 144; n <= 255; n++)
        lens[n] = 9;
    for (n = 256; n <= 279; n++)
        lens[n] = 7;
    for (n = 280; n <= 287; n++)
        lens[n] = 8;
    for (n = 0; n < 32; n++)
        lens[288 + n] = 5;

    // Build lit/dist trees.
    s->tlit = build(s, s->litcodes, lens, 288);
    s->tdist = build(s, s->distcodes, lens + 288, 32);
}

static void dynamic(State *s) {
    int n, i, nlit, ndist, nlen;
    unsigned char lenlens[19] = { 0 }, lens[288 + 32];
    nlit = 257 + bits(s, 5);
    ndist = 1 + bits(s, 5);
    nlen = 4 + bits(s, 4);
    for (n = 0; n < nlen; n++)
        lenlens[(unsigned char)order[n]] = (unsigned char)bits(s, 3);

    // Build the tree for decoding code lengths.
    s->tlen = build(s, s->lencodes, lenlens, 19);

    // Decode code lengths.
    for (n = 0; n < nlit + ndist;) {
        int sym = decode(s, s->lencodes, s->tlen);
        switch (sym) {
            case 16:
                for (i = 3 + bits(s, 2); i; i--, n++)
                    lens[n] = lens[n - 1];
                break;
            case 17:
                for (i = 3 + bits(s, 3); i; i--, n++)
                    lens[n] = 0;
                break;
            case 18:
                for (i = 11 + bits(s, 7); i; i--, n++)
                    lens[n] = 0;
                break;
            default:
                lens[n++] = (unsigned char)sym;
                break;
        }
    }

    // Build lit/dist trees.
    s->tlit = build(s, s->litcodes, lens, nlit);
    s->tdist = build(s, s->distcodes, lens + nlit, ndist);
}

static int inflate(void *out, unsigned outlen, const void *in, unsigned inlen) {
    int last;
    State *s = calloc(1, sizeof(State));

    // We assume we can buffer 2 extra bytes from off the end of 'in'.
    s->in = (unsigned char*)in;
    s->inend = s->in + inlen + 2;
    s->out = (unsigned char*)out;
    s->outend = s->out + outlen;
    s->bits = 0;
    s->count = 0;
    bits(s, 0);

    if (setjmp(s->jmp) == 1) {
        free(s);
        return 0;
    }

    do {
        last = bits(s, 1);
        switch (bits(s, 2)) {
            case 0:
                stored(s);
                break;
            case 1:
                fixed(s);
                block(s);
                break;
            case 2:
                dynamic(s);
                block(s);
                break;
            case 3:
                INFLATE_FAIL();
        }
    } while (!last);

    free(s);
    return 1;
}

static ezImage* load_png(PNG *png) {
    const unsigned char *ihdr, *idat, *plte, *trns, *first;
    int trnsSize = 0;
    int depth, ctype, bipp;
    int datalen = 0;
    ezImage *img = NULL;
    unsigned char *data = NULL, *out;
    
    PNG_CHECK(memcmp(png->p, "\211PNG\r\n\032\n", 8) == 0);  // PNG signature
    png->p += 8;
    first = png->p;
    
    // Read IHDR
    ihdr = find(png, "IHDR", 13);
    PNG_CHECK(ihdr);
    depth = ihdr[8];
    ctype = ihdr[9];
    switch (ctype) {
        case 0:
            bipp = depth;
            break;  // greyscale
        case 2:
            bipp = 3 * depth;
            break;  // RGB
        case 3:
            bipp = depth;
            break;  // paletted
        case 4:
            bipp = 2 * depth;
            break;  // grey+alpha
        case 6:
            bipp = 4 * depth;
            break;  // RGBA
        default:
            PNG_FAIL();
    }
    
    // Allocate bitmap (+1 width to save room for stupid PNG filter bytes)
    img = ezImageNew(get32(ihdr + 0) + 1, get32(ihdr + 4));
    PNG_CHECK(img->buf);
    img->w--;
    
    // We support 8-bit color components and 1, 2, 4 and 8 bit palette formats.
    // No interlacing, or wacky filter types.
    PNG_CHECK((depth != 16) && ihdr[10] == 0 && ihdr[11] == 0 && ihdr[12] == 0);
    
    // Join IDAT chunks.
    for (idat = find(png, "IDAT", 0); idat; idat = find(png, "IDAT", 0)) {
        unsigned len = get32(idat - 8);
        data = realloc(data, datalen + len);
        if (!data)
            break;
        
        memcpy(data + datalen, idat, len);
        datalen += len;
    }
    
    // Find palette.
    png->p = first;
    plte = find(png, "PLTE", 0);
    
    // Find transparency info.
    png->p = first;
    trns = find(png, "tRNS", 0);
    if (trns) {
        trnsSize = get32(trns - 8);
    }
    
    PNG_CHECK(data && datalen >= 6);
    PNG_CHECK((data[0] & 0x0f) == 0x08  // compression method (RFC 1950)
          && (data[0] & 0xf0) <= 0x70   // window size
          && (data[1] & 0x20) == 0);    // preset dictionary present
    
    out = (unsigned char*)img->buf + outsize(img, 32) - outsize(img, bipp);
    PNG_CHECK(inflate(out, outsize(img, bipp), data + 2, datalen - 6));
    PNG_CHECK(unfilter(img->w, img->h, bipp, out));
    
    if (ctype == 3) {
        PNG_CHECK(plte);
        depalette(img->w, img->h, out, img->buf, bipp, plte, trns, trnsSize);
    } else {
        PNG_CHECK(bipp % 8 == 0);
        convert(bipp / 8, img->w, img->h, out, img->buf, trns);
    }
    
    free(data);
    return img;
    
err:
    if (data)
        free(data);
    ezImageFree(img);
    return NULL;
}

static unsigned char* read_file(const char *path, size_t *sizeOfFile) {
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

ezImage* ezImageLoadFromPath(const char *path) {
    unsigned char *data = NULL;
    size_t sizeOfData = 0;
    if (!(data = read_file(path, &sizeOfData)) && sizeOfData > 0)
        return 0;
    ezImage *result = ezImageLoadFromMemory((void*)data, sizeOfData);
    free(data);
    return result;
}

ezImage* ezImageLoadFromMemory(const void *data, size_t sizeOfData) {
    PNG png = {
        .p = (unsigned char*)data,
        .end = (unsigned char*)data + sizeOfData
    };
    return load_png(&png);
}

typedef struct {
    unsigned crc, adler, bits, prev, runlen;
    FILE *out;
    unsigned crcTable[256];
} Save;

static const unsigned crctable[16] = { 0,          0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
                                       0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
                                       0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };

static void put(Save *s, unsigned v) {
    fputc(v, s->out);
    s->crc = (s->crc >> 4) ^ crctable[(s->crc & 15) ^ (v & 15)];
    s->crc = (s->crc >> 4) ^ crctable[(s->crc & 15) ^ (v >> 4)];
}

static void updateAdler(Save *s, unsigned v) {
    unsigned s1 = s->adler & 0xffff, s2 = (s->adler >> 16) & 0xffff;
    s1 = (s1 + v) % 65521;
    s2 = (s2 + s1) % 65521;
    s->adler = (s2 << 16) + s1;
}

static void put32(Save *s, unsigned v) {
    put(s, (v >> 24) & 0xff);
    put(s, (v >> 16) & 0xff);
    put(s, (v >> 8) & 0xff);
    put(s, v & 0xff);
}

void putbits(Save *s, unsigned data, unsigned bitcount) {
    while (bitcount--) {
        unsigned prev = s->bits;
        s->bits = (s->bits >> 1) | ((data & 1) << 7);
        data >>= 1;
        if (prev & 1) {
            put(s, s->bits);
            s->bits = 0x80;
        }
    }
}

void putbitsr(Save *s, unsigned data, unsigned bitcount) {
    while (bitcount--)
        putbits(s, data >> bitcount, 1);
}

static void begin(Save *s, const char *id, unsigned len) {
    put32(s, len);
    s->crc = 0xffffffff;
    put(s, id[0]);
    put(s, id[1]);
    put(s, id[2]);
    put(s, id[3]);
}

static void literal(Save *s, unsigned v) {
    // Encode a literal/length using the built-in tables.
    // Could do better with a custom table but whatever.
    if (v < 144)
        putbitsr(s, 0x030 + v - 0, 8);
    else if (v < 256)
        putbitsr(s, 0x190 + v - 144, 9);
    else if (v < 280)
        putbitsr(s, 0x000 + v - 256, 7);
    else
        putbitsr(s, 0x0c0 + v - 280, 8);
}

static void encodelen(Save *s, unsigned code, unsigned bits, unsigned len) {
    literal(s, code + (len >> bits));
    putbits(s, len, bits);
    putbits(s, 0, 5);
}

static void endrun(Save *s) {
    s->runlen--;
    literal(s, s->prev);

    if (s->runlen >= 67)
        encodelen(s, 277, 4, s->runlen - 67);
    else if (s->runlen >= 35)
        encodelen(s, 273, 3, s->runlen - 35);
    else if (s->runlen >= 19)
        encodelen(s, 269, 2, s->runlen - 19);
    else if (s->runlen >= 11)
        encodelen(s, 265, 1, s->runlen - 11);
    else if (s->runlen >= 3)
        encodelen(s, 257, 0, s->runlen - 3);
    else
        while (s->runlen--)
            literal(s, s->prev);
}

static void encodeByte(Save *s, unsigned char v) {
    updateAdler(s, v);

    // Simple RLE compression. We could do better by doing a search
    // to find matches, but this works pretty well TBH.
    if (s->prev == v && s->runlen < 115) {
        s->runlen++;
    } else {
        if (s->runlen)
            endrun(s);

        s->prev = v;
        s->runlen = 1;
    }
}

static void savePngHeader(Save *s, ezImage *img) {
    fwrite("\211PNG\r\n\032\n", 8, 1, s->out);
    begin(s, "IHDR", 13);
    put32(s, img->w);
    put32(s, img->h);
    put(s, 8);  // bit depth
    put(s, 6);  // RGBA
    put(s, 0);  // compression (deflate)
    put(s, 0);  // filter (standard)
    put(s, 0);  // interlace off
    put32(s, ~s->crc);
}

static long savePngData(Save *s, ezImage *img, long dataPos) {
    int x, y;
    long dataSize;
    begin(s, "IDAT", 0);
    put(s, 0x08);      // zlib compression method
    put(s, 0x1d);      // zlib compression flags
    putbits(s, 3, 3);  // zlib last block + fixed dictionary
    for (y = 0; y < img->h; y++) {
        int *row = &img->buf[y * img->w];
        int prev = RGBA1(0, 0);

        encodeByte(s, 1);  // sub filter
        for (x = 0; x < img->w; x++) {
            encodeByte(s, Rgba(row[x]) - Rgba(prev));
            encodeByte(s, rGba(row[x]) - rGba(prev));
            encodeByte(s, rgBa(row[x]) - rgBa(prev));
            encodeByte(s, rgbA(row[x]) - rgbA(prev));
            prev = row[x];
        }
    }
    endrun(s);
    literal(s, 256);  // terminator
    while (s->bits != 0x80)
        putbits(s, 0, 1);
    put32(s, s->adler);
    dataSize = (ftell(s->out) - dataPos) - 8;
    put32(s, ~s->crc);
    return dataSize;
}

int ezImageSave(ezImage *img, const char *path) {
    FILE *out = fopen(path, "wb");
    if (!out)
        return 0;
    
    Save s;
    s.out = out;
    s.adler = 1;
    s.bits = 0x80;
    s.prev = 0xffff;
    s.runlen = 0;
    
    savePngHeader(&s, img);
    long dataPos = ftell(s.out);
    long dataSize = savePngData(&s, img, dataPos);
    
    // End chunk.
    begin(&s, "IEND", 0);
    put32(&s, ~s.crc);
    
    // Write back payload size.
    fseek(out, dataPos, SEEK_SET);
    put32(&s, dataSize);
    
    long err = ferror(out);
    fclose(out);
    return !err;
}
#endif // EZIMAGE_IMPLEMENTATION
