#define EZIMAGE_IMPLEMENTATION
#include "ezimage.h"

int main(int argc, const char *argv[]) {
    ezImage *image = ezImageNew(256, 256);
    for (int x = 0; x < 256; x++)
        for (int y = 0; y < 256; y++)
            ezImagePSet(image, x, y, RGB1(x ^ y));
    ezImageDrawString(image, "THIS IS JUST A TEST", 0, 0, 0xFFFF0000);
    ezImageSave(image, "test_xor.png");
    ezImageFree(image);
    return 0;
}
