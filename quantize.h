//https://www.rosettacode.org/wiki/Color_quantization/C
//Content is available under GNU Free Documentation License 1.2 unless otherwise noted.

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct {
    int w, h;
    unsigned char *pix;
} image_t, *image;

typedef struct oct_node_t oct_node_t, *oct_node;
struct oct_node_t{
    int64_t r, g, b; /* sum of all child node colors */
    int count, heap_idx;
    unsigned char n_kids, kid_idx, flags, depth;
    oct_node kids[8], parent;
};

typedef struct {
    int alloc, n;
    oct_node* buf;
} node_heap;


int color_quant(image im, int n_colors, int dither);

#endif // QUANTIZE_H
