#define EZIMAGE_IMPLEMENTATION
#include "ezimage.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* FileExt(const char *path) {
    char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

static char* RemoveExt(const char* path) {
    char *resut = strdup(path);
    char *dot = strrchr(resut, '.');
    if (dot)
        *dot = '\0';
    return resut;
}

int main(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(FileExt(argv[i]), "qoi", 3)) {
            fprintf(stderr, "ERROR: Skpping \"%s\", invalid file extension!\n", argv[i]);
            continue;
        }
        
        qoi_desc desc;
        unsigned char *data = qoi_read(argv[i], &desc, 0);
        if (!data) {
            fprintf(stderr, "ERROR: qoi_read() failed for \"%s\"\n", argv[i]);
            continue;
        }
        
        int n = desc.channels;
        int w = desc.width;
        int h = desc.height;
        
        ezImage *a = ezImageNew(w, h);
        for (int x = 0; x < w; x++)
            for (int y = 0; y < h; y++) {
                unsigned char *p = data + (x + w * y) * n;
                ezImagePSet(a, x, y, RGBA(p[0], p[1], p[2], n == 4 ? p[3] : 255));
            }
        
        ezImage *b = ezImageRotate(a, 90.f);
        ezImageFree(a);
        char *fname = RemoveExt(argv[i]);
        char *path = malloc(strlen(fname) + 9 * sizeof(char));
        sprintf(path, "%s_copy.qoi", fname);
        free(fname);
        
        char *out_data = malloc(b->w * b->h * 4 * sizeof(char));
        for (int i = 0, x = 0; x < b->h; x++)
            for (int y = 0; y < b->w; y++) {
                int c = ezImagePGet(b, y, x);
                out_data[i++] = Rgba(c);
                out_data[i++] = rGba(c);
                out_data[i++] = rgBa(c);
                out_data[i++] = rgbA(c);
            }
        
        qoi_desc out_desc = {
            .width = b->w,
            .height = b->h,
            .channels = 4,
            .colorspace = QOI_SRGB
        };
        qoi_write(path, out_data, &out_desc);
        
        ezImageFree(b);
        free(out_data);
        free(path);
    }
    return 0;
}
