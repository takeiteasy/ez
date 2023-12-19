#define EZIMAGE_IMPLEMENTATION
#include "ezimage.h"
#define MSF_GIF_IMPL
#include "msf_gif.h"
#include "gif_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int delay, sizeOfFrames, currentFrame, w, h;
    ezImage **frames;
} Gif;

#pragma pack(push, 1)
typedef struct {
  void *data, *pict, *prev;
  unsigned long size, last;
  Gif* out;
} GifData;
#pragma pack(pop)

static void GifLoadFrame(void *data, struct GIF_WHDR* whdr) {
    uint32_t *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
    GifData *gif = (GifData*)data;

#define BGRA(i) \
    ((whdr->bptr[i] == whdr->tran) ? 0 : ((uint32_t)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE) ? 8 : 16)) | (uint32_t)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE) ? 16 : 8)) | (uint32_t)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE) ? 24 : 0)) | ((GIF_BIGE) ? 0xFF : 0xFF000000)))

    if (!whdr->ifrm) {
        gif->out->delay = (int)whdr->time;
        gif->out->w = (int)whdr->xdim;
        gif->out->h = (int)whdr->ydim;
        gif->out->sizeOfFrames = (int)whdr->nfrm;
        gif->out->currentFrame = 0;
        ddst = (uint32_t)(whdr->xdim * whdr->ydim);
        gif->pict = malloc(ddst * sizeof(uint32_t));
        gif->prev = malloc(ddst * sizeof(uint32_t));
        gif->out->frames = malloc(gif->out->sizeOfFrames * sizeof(ezImage*));
    }

    pict = (uint32_t*)gif->pict;
    ddst = (uint32_t)(whdr->xdim * whdr->fryo + whdr->frxo);
    ifin = (!(iter = (whdr->intr) ? 0 : 4)) ? 4 : 5; /** interlacing support **/
    for (dsrc = (uint32_t)-1; iter < ifin; iter++)
        for (yoff = 16U >> ((iter > 1) ? iter : 1), y = (8 >> iter) & 7; y < (uint32_t)whdr->fryd; y += yoff)
            for (x = 0; x < (uint32_t)whdr->frxd; x++)
                if (whdr->tran != (long)whdr->bptr[++dsrc])
                    pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(dsrc);

    int this = (int)whdr->ifrm;
    gif->out->frames[this] = ezImageNew(gif->out->w, gif->out->h);
    memcpy(gif->out->frames[this]->buf, pict, whdr->xdim * whdr->ydim * sizeof(uint32_t));

    if ((whdr->mode == GIF_PREV) && !gif->last) {
        whdr->frxd = whdr->xdim;
        whdr->fryd = whdr->ydim;
        whdr->mode = GIF_BKGD;
        ddst = 0;
    } else {
        gif->last = (whdr->mode == GIF_PREV) ? gif->last : (unsigned long)(whdr->ifrm + 1);
        pict = (uint32_t*)((whdr->mode == GIF_PREV) ? gif->pict : gif->prev);
        prev = (uint32_t*)((whdr->mode == GIF_PREV) ? gif->prev : gif->pict);
        for (x = (uint32_t)(whdr->xdim * whdr->ydim); --x;
             pict[x - 1] = prev[x - 1]);
    }

    if (whdr->mode == GIF_BKGD) /** cutting a hole for the next frame **/
        for (whdr->bptr[0] = (uint8_t)((whdr->tran >= 0) ? whdr->tran : whdr->bkgd), y = 0, pict = (uint32_t*)gif->pict; y < (uint32_t)whdr->fryd; y++)
            for (x = 0; x < (uint32_t)whdr->frxd; x++)
                pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(0);
#undef BGRA
}

static void LoadGif(Gif *g, const char *path) {
    FILE *fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    rewind(fp);

    unsigned char *data = calloc(length + 1, sizeof(unsigned char));
    fread(data, 1, length, fp);
    fclose(fp);

    GifData tmp;
    tmp.data = data;
    tmp.size = length;
    tmp.out = g;
    GIF_Load(data, length, GifLoadFrame, 0, (void *)&tmp, 0L);

    free(tmp.data);
    free(tmp.prev);
    free(tmp.pict);
}

static void SaveGif(Gif *g, int depth, const char *path) {
    msf_gif_bgra_flag = 1;
    MsfGifState state;
    msf_gif_begin(&state, g->w, g->h);
    for (int i = 0; i < g->sizeOfFrames; i++)
        msf_gif_frame(&state, (uint8_t*)g->frames[i]->buf, g->delay, depth, g->w * 4);
    MsfGifResult result = msf_gif_end(&state);
    if (result.data) {
        FILE *fp = fopen(path, "wb");
        fwrite(result.data, result.dataSize, 1, fp);
        fclose(fp);
    }
    msf_gif_free(result);
}

static void DestroyGif(Gif *g) {
    for (int i = 0; i < g->sizeOfFrames; i++)
        ezImageFree(g->frames[i]);
}

#define WIDTH 256
#define HEIGHT 256
#define FRAMES 10

int main(int argc, const char *argv[]) {
    srand((unsigned int)time(NULL));
    
    Gif gif = {
        .delay = 1,
        .sizeOfFrames = FRAMES,
        .w = WIDTH,
        .h = HEIGHT
    };
    gif.frames = malloc(sizeof(ezImage*) * 10);
    
    for (int i = 0; i < FRAMES; i++) {
        gif.frames[i] = ezImageNew(WIDTH, HEIGHT);
        ezImageFill(gif.frames[i], RGB(rand()%256, rand()%256, rand()%256));
    }
    
    SaveGif(&gif, 16, "test.gif");
    
    DestroyGif(&gif);
    
    LoadGif(&gif, "test.gif");
    
    for (int i = 0; i < FRAMES; i++) {
        char path[16];
        sprintf(path, "test_%d.png", i);
        ezImageSave(gif.frames[i], path);
    }
    
    DestroyGif(&gif);
    
    return 0;
}
