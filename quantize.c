//https://www.rosettacode.org/wiki/Color_quantization/C
//Content is available under GNU Free Documentation License 1.2 unless otherwise noted.

#include <quantize.h>
 
#define ON_INHEAP	1
 
int cmp_node(oct_node a, oct_node b)
{
	if (a->n_kids < b->n_kids) return -1;
	if (a->n_kids > b->n_kids) return 1;
 
	int ac = a->count >> a->depth;
	int bc = b->count >> b->depth;
	return ac < bc ? -1 : ac > bc;
}
 
void down_heap(node_heap *h, oct_node p)
{
	int n = p->heap_idx, m;
	while (1) {
		m = n * 2;
		if (m >= h->n) break;
		if (m + 1 < h->n && cmp_node(h->buf[m], h->buf[m + 1]) > 0) m++;
 
		if (cmp_node(p, h->buf[m]) <= 0) break;
 
		h->buf[n] = h->buf[m];
		h->buf[n]->heap_idx = n;
		n = m;
	}
	h->buf[n] = p;
	p->heap_idx = n;
}
 
void up_heap(node_heap *h, oct_node p)
{
	int n = p->heap_idx;
	oct_node prev;
 
	while (n > 1) {
		prev = h->buf[n / 2];
		if (cmp_node(p, prev) >= 0) break;
 
		h->buf[n] = prev;
		prev->heap_idx = n;
		n /= 2;
	}
	h->buf[n] = p;
	p->heap_idx = n;
}
 
void heap_add(node_heap *h, oct_node p)
{
	if ((p->flags & ON_INHEAP)) {
		down_heap(h, p);
		up_heap(h, p);
		return;
	}
 
	p->flags |= ON_INHEAP;
	if (!h->n) h->n = 1;
	if (h->n >= h->alloc) {
        while (h->n >= h->alloc) h->alloc += 1024;
		h->buf = realloc(h->buf, sizeof(oct_node) * h->alloc);
	}
 
	p->heap_idx = h->n;
	h->buf[h->n++] = p;
	up_heap(h, p);
}
 
oct_node pop_heap(node_heap *h)
{
	if (h->n <= 1) return 0;
 
	oct_node ret = h->buf[1];
	h->buf[1] = h->buf[--h->n];
 
	h->buf[h->n] = 0;
 
	h->buf[1]->heap_idx = 1;
	down_heap(h, h->buf[1]);
 
	return ret;
}
 
static oct_node pool = 0;
static int len = 0;
oct_node node_new(unsigned char idx, unsigned char depth, oct_node p, int bForce)
{
    if (bForce)
    {
        len = 0;
        pool = 0;
    }
    //if (len <= 1) {
    if (len < 1) {
        oct_node p = calloc(sizeof(oct_node_t), 2048*32);
		p->parent = pool;
		pool = p;
        len = 2048*32-1;
	}
 
	oct_node x = pool + len--;
	x->kid_idx = idx;
	x->depth = depth;
	x->parent = p;
	if (p) p->n_kids++;
	return x;
}
 
void node_free()
{
	oct_node p;
	while (pool) {
		p = pool->parent;
		free(pool);
		pool = p;
	}
    len = 0;
    pool = 0;
}
 
oct_node node_insert(oct_node root, unsigned char *pix)
{
	unsigned char i, bit, depth = 0;
 
	for (bit = 1 << 7; ++depth < 8; bit >>= 1) {
		i = !!(pix[1] & bit) * 4 + !!(pix[0] & bit) * 2 + !!(pix[2] & bit);
		if (!root->kids[i])
            root->kids[i] = node_new(i, depth, root,0);
 
		root = root->kids[i];
	}
 
    /*root->r += pix[0];
	root->g += pix[1];
    root->b += pix[2];*/
    root->r += pix[0]/8;
    root->g += pix[1]/8;
    root->b += pix[2]/8;
    /*root->r += (pix[0]+2)&0xF8;
    root->g += (pix[1]+2)&0xF8;
    root->b += (pix[2]+2)&0xF8;*/
    root->count++;
	return root;
}
 
oct_node node_fold(oct_node p)
{
	if (p->n_kids) abort();
	oct_node q = p->parent;
	q->count += p->count;
 
	q->r += p->r;
	q->g += p->g;
	q->b += p->b;
    /*q->r = (q->r+2)&0xF8;
    q->g = (q->g+2)&0xF8;
    q->b = (q->b+2)&0xF8;*/
    q->n_kids --;
	q->kids[p->kid_idx] = 0;
	return q;
}
 
void color_replace(oct_node root, unsigned char *pix)
{
	unsigned char i, bit;
 
	for (bit = 1 << 7; bit; bit >>= 1) {
		i = !!(pix[1] & bit) * 4 + !!(pix[0] & bit) * 2 + !!(pix[2] & bit);
		if (!root->kids[i]) break;
		root = root->kids[i];
	}
 

    /* pix[0] = root->r;
     pix[1] = root->g;
     pix[2] = root->b;*/

     pix[0] = root->r*8;
     pix[1] = root->g*8;
     pix[2] = root->b*8;

    /*pix[0] = (root->r+2)&0xF8;
    pix[1] = (root->g+2)&0xF8;
    pix[2] = (root->b+2)&0xF8;*/
}

oct_node nearest_color(int *v, node_heap *h) {
    int i;
    int diff, max = 100000000;
    oct_node o = 0;
    for (i = 1; i < h->n; i++) {
        diff =	  3 * abs(h->buf[i]->r - v[0])
            + 5 * abs(h->buf[i]->g - v[1])
            + 2 * abs(h->buf[i]->b - v[2]);
        if (diff < max) {
            max = diff;
            o = h->buf[i];
        }
    }
    return o;
}

/*int calculate_unique_colors(node_heap *h) {
    int colors[8192];
    int num = 0;
    int color;
    int bfound;
    int j;
    oct_node o = 0;
    for (int i = 1; i < h->n; i++) {
        color = 0x10000 * h->buf[i]->r +0x100 * h->buf[i]->g +h->buf[i]->b;
        bfound = 0;
        j=0;
        while ( (0 == bfound) && (j<num) )
        {
            if (colors[j] == color) bfound = 1;
            j++;
        }
        if (0 == bfound)
        {
            colors[num] = color;
            num++;
        }
    }
    return num;
}*/

#define POS(i, j) (3 * ((i) * im->w + (j)))
//#define clamp(x, i) if (x[i] > 255) x[i] = 255; if (x[i] < 0) x[i] = 0
#define clamp(x, i) if (x[i] > 31) x[i] = 31; if (x[i] < 0) x[i] = 0
#define C10 7
#define C01 5
#define C11 2
#define C00 1
#define CTOTAL (C00 + C11 + C10 + C01)

int error_diffuse(image im, node_heap *h)
{
	int i, j;
    int *npx = calloc(sizeof(int), im->h * im->w * 4);
    int *px;
	int v[3];
	unsigned char *pix = im->pix;
	oct_node nd;
    int colors[8192*4];
    for (i=0;i<8192*4;i++) colors[i] = 0;
    int colors_num = 0;
    int color;
    int bfound;
 
    for (px = npx, i = 0; i < im->h; i++) {
        //for (j = 0; j < im->w; j++, pix += 3, px += 3) {
        for (j = 0; j < im->w; j++, pix += 4, px += 3) {
            px[0] = (int)pix[0] * CTOTAL /8;
            px[1] = (int)pix[1] * CTOTAL /8;
            px[2] = (int)pix[2] * CTOTAL /8;
		}
	}
	pix = im->pix;
    for (px = npx, i = 0; i < im->h; i++) {
        //for (j = 0; j < im->w; j++, pix += 3, px += 3) {
        for (j = 0; j < im->w; j++, pix += 4, px += 3) {
            px[0] /= CTOTAL;
			px[1] /= CTOTAL;
			px[2] /= CTOTAL;
			clamp(px, 0); clamp(px, 1); clamp(px, 2);
 
            nd = nearest_color(px,h);
 
			v[0] = px[0] - nd->r;
			v[1] = px[1] - nd->g;
			v[2] = px[2] - nd->b;
 
            pix[0] = nd->r*8; pix[1] = nd->g*8; pix[2] = nd->b*8;
            //pix[0] = nd->r; pix[1] = nd->g; pix[2] = nd->b;
            if (j < im->w - 1) {
                npx[POS(i, j+1) + 0] += v[0] * C10;
                npx[POS(i, j+1) + 1] += v[1] * C10;
                npx[POS(i, j+1) + 2] += v[2] * C10;
			}
			if (i >= im->h - 1) continue;
 
            npx[POS(i+1, j) + 0] += v[0] * C01;
            npx[POS(i+1, j) + 1] += v[1] * C01;
            npx[POS(i+1, j) + 2] += v[2] * C01;
 
			if (j < im->w - 1) {
                npx[POS(i+1, j+1) + 0] += v[0] * C11;
                npx[POS(i+1, j+1) + 1] += v[1] * C11;
                npx[POS(i+1, j+1) + 2] += v[2] * C11;
			}
			if (j) {
                npx[POS(i+1, j-1) + 0] += v[0] * C00;
                npx[POS(i+1, j-1) + 1] += v[1] * C00;
                npx[POS(i+1, j-1) + 2] += v[2] * C00;
			}

            //colors calc
            color = 0x10000 * pix[0] + 0x100 * pix[1] + pix[2];
            bfound = 0;
            int k=0;
            while ( (0 == bfound) && (k<colors_num) )
            {
                if (colors[k] == color) bfound = 1;
                k++;
            }
            if (0 == bfound)
            {
                colors[colors_num] = color;
                colors_num++;
            }
		}
	}
	free(npx);
    return colors_num;
}
 
int color_quant(image im, int n_colors, int dither)
{
	int i;
	unsigned char *pix = im->pix;
    node_heap heap = { 0, 0, 0 };

    int current_colors=0;
    //int current_colors_limit=n_colors;
    int colors[8192*4];
    int colors_num = 0;
    int color;
    int bfound;

    oct_node root = node_new(0, 0, 0, 1);
    oct_node got;
    for (i = 0; i < im->w * im->h; i++, pix += 3)
        heap_add(&heap, node_insert(root, pix));

    //int colors = calculate_unique_colors(&heap);
    //printf("Start colors number: %i \n",colors);
    while (heap.n > n_colors + 1)
    //while (colors > n_colors*10)
    {
        heap_add(&heap, node_fold(pop_heap(&heap)));
        //colors = calculate_unique_colors(&heap);
    }
    //printf("End colors number: %i \n",colors);

    double c;
    for (i = 1; i < heap.n; i++) {
        got = heap.buf[i];
        c = got->count;
        got->r = got->r / c + .5;
        got->g = got->g / c + .5;
        got->b = got->b / c + .5;

        /*got->r = round((got->r / c )/8)*8;
        got->g = round((got->g / c )/8)*8;
        got->b = round((got->b / c )/8)*8;*/
    }

    if (dither)
    {
        current_colors = error_diffuse(im, &heap);
    }
    else
    {
        colors_num = 0;
        for (i = 0, pix = im->pix; i < im->w * im->h; i++, pix += 4)
        {
            color_replace(root, pix);
            //colors calc
            color = 0x10000 * pix[0] + 0x100 * pix[1] + pix[2];
            bfound = 0;
            int k=0;
            while ( (0 == bfound) && (k<colors_num) )
            {
                if (colors[k] == color) bfound = 1;
                k++;
            }
            if (0 == bfound)
            {
                colors[colors_num] = color;
                colors_num++;
            }
        }
        current_colors = colors_num;
    }

    //folding into zero
    while (heap.n > 2)
    {
        pop_heap(&heap);
    }

    node_free();
    free(heap.buf);

    return current_colors;
}
