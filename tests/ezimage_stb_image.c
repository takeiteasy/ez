#define EZIMAGE_IMPLEMENTATION
#include "ezimage.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
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

static void ezImageSaveEx(ezImage *img, const char *path) {
    char *ext = FileExt(path);
    size_t sizeOfData = img->w * img->h * 4 * sizeof(char);
    char *data = malloc(sizeOfData);
    for (int i = 0, x = 0; x < img->h; x++)
        for (int y = 0; y < img->w; y++) {
            int c = ezImagePGet(img, y, x);
            data[i++] = Rgba(c);
            data[i++] = rGba(c);
            data[i++] = rgBa(c);
            data[i++] = rgbA(c);
        }
    
    if (!strncmp(ext, "png", 3)) {
        stbi_write_png(path, img->w, img->h, 4, data, 0);
    } else if (!strncmp(ext, "bmp", 3)) {
        stbi_write_bmp(path, img->w, img->h, 4, data);
    } else if (!strncmp(ext, "tga", 3)) {
        stbi_write_tga(path, img->w, img->h, 4, data);
    } else if (!strncmp(ext, "jpg", 3)) {
        stbi_write_jpg(path, img->w, img->h, 4, data, 80);
    } else if (!strncmp(ext, "qoi", 3)) {
        qoi_desc desc = {
            .width = img->w,
            .height = img->h,
            .channels = 4,
            .colorspace = QOI_SRGB
        };
        qoi_write(path, data, &desc);
    } else {
        FILE *fh = fopen(path, "wb");
        assert(fh);
        fwrite(data, sizeOfData, 1, fh);
        fclose(fh);
    }

    free(data);
}

static ezImage* ezImageLoadFromPathEx(const char *path) {
    int w, h, n;
    unsigned char *data = NULL;
    char *ext = FileExt(path);
    if (!strncmp(ext, "qoi", 3)) {
        qoi_desc desc;
        data = qoi_read(path, &desc, 0);
        n = desc.channels;
        w = desc.width;
        h = desc.height;
    } else {
        data = stbi_load(path, &w, &h, &n, 4);
    }
    
    ezImage *result = ezImageNew(w, h);
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++) {
            unsigned char *p = data + (x + w * y) * n;
            ezImagePSet(result, x, y, RGBA(p[0], p[1], p[2], n == 4 ? p[3] : 255));
        }
    if (data)
        free(data);
    return result;
}

int main(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        ezImage *a = ezImageLoadFromPath(argv[i]);
        ezImage *b = ezImageRotate(a, 90.f);
        ezImageFree(a);
        
        char *ext = FileExt(argv[i]);
        char *fname = RemoveExt(argv[i]);
        char *path = malloc(strlen(fname) + 5 + strlen(ext) * sizeof(char));
        sprintf(path, "%s_copy.%s", fname, ext);
        free(fname);
        
        ezImageSaveEx(b, path);
        free(path);
        ezImageFree(b);
    }
    return 0;
}
