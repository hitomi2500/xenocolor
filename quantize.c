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
 
    root->r += pix[0];
	root->g += pix[1];
    root->b += pix[2];
    /*root->r += pix[0]/8;
    root->g += pix[1]/8;
    root->b += pix[2]/8;*/
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
 

     pix[0] = root->r;
     pix[1] = root->g;
     pix[2] = root->b;

     /*pix[0] = root->r*8;
     pix[1] = root->g*8;
     pix[2] = root->b*8;*/

    /*pix[0] = (root->r+2)&0xF8;
    pix[1] = (root->g+2)&0xF8;
    pix[2] = (root->b+2)&0xF8;*/
}

void color_replace_stupid(oct_node * stupid_buf, int stupid_buf_n, unsigned char *pix)
{
    int diff = 100500000;
    int currdiff = 0;
    int ind = 0;

    for (int i=0; i< stupid_buf_n; i++)
    {
        currdiff = (stupid_buf[i]->r - pix[0])*(stupid_buf[i]->r - pix[0]) +
                (stupid_buf[i]->g - pix[1])*(stupid_buf[i]->g - pix[1]) +
                (stupid_buf[i]->b - pix[2])*(stupid_buf[i]->b - pix[2]);
        if ( currdiff < diff )
        {
            ind = i;
            diff = currdiff;
        }
    }

     pix[0] = stupid_buf[ind]->r;
     pix[1] = stupid_buf[ind]->g;
     pix[2] = stupid_buf[ind]->b;
}

int color_find_stupid(oct_node * stupid_buf, int stupid_buf_n, unsigned char *pix)
{
    int diff = 100500000;
    int currdiff = 0;
    int ind = 0;

    for (int i=0; i< stupid_buf_n; i++)
    {
        currdiff = (stupid_buf[i]->r - pix[0])*(stupid_buf[i]->r - pix[0]) +
                (stupid_buf[i]->g - pix[1])*(stupid_buf[i]->g - pix[1]) +
                (stupid_buf[i]->b - pix[2])*(stupid_buf[i]->b - pix[2]);
        if ( currdiff < diff )
        {
            ind = i;
            diff = currdiff;
        }
    }

    return ind;
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
 
void debug_dump_colors_recursive(FILE * _f, oct_node node)
{
    char buf[256];
    sprintf(buf,"%i %i %i\r\n",node->r, node->g, node->b);
    fwrite(buf,strlen(buf),1,_f);
    for (int i = 0; i< node->n_kids; i++)
    {
        if (node->kids[i] != NULL)
            debug_dump_colors_recursive(_f,node->kids[i]);
    }

}

int color_quant(image im, int n_colors, int dither)
{
    int i,j;
	unsigned char *pix = im->pix;
    node_heap heap = { 0, 0, 0 };

    int current_colors=0;
    //int current_colors_limit=n_colors;
    int colors[8192*4];
    char buf[256];
    int colors_num = 0;
    int color;
    int bfound;
    int stupid_buf_n = 0;

    oct_node root = node_new(0, 0, 0, 1);
    oct_node got;
    for (i = 0; i < im->w * im->h; i++, pix += 3)
        heap_add(&heap, node_insert(root, pix));

    printf("TEST NOTE 10 : heap.n = %i \n",heap.n);

    //moving to linear array instead of this ugly shit
    oct_node* stupid_buf = malloc(sizeof(oct_node)*heap.n);
    //oct_node stupid_buf[3000];
    for (i = 1; i < heap.n; i++)
    {
        stupid_buf[i] = heap.buf[i];
        /*got = heap.buf[i];
        stupid_buf[i].count = got->count;
        stupid_buf[i].r = got->r;
        stupid_buf[i].g = got->g;
        stupid_buf[i].b = got->b;
        stupid_buf[i].depth = got->depth;
        stupid_buf[i].flags = got->flags;
        stupid_buf[i].heap_idx = got->heap_idx;
        stupid_buf[i].kid_idx = got->kid_idx;
        for (int i=0;i<8;i++)
            stupid_buf[i].kids[i] = got->kids[i];
        stupid_buf[i].n_kids = got->n_kids;
        stupid_buf[i].parent = got->parent;*/
    }
    stupid_buf[0] = heap.buf[1];//missing zero shit

    stupid_buf_n = heap.n;

    //killing zero-count trash (why it's there anyway?)
    for (j = 0; j < stupid_buf_n; j++) {
        if (stupid_buf[j]->count <= 0)
        {
            //removing node j
                    for (int k=j; k< stupid_buf_n; k++)
                        stupid_buf[k] = stupid_buf[k+1];
                    stupid_buf_n--;
                    j=0;
        }
    }

    printf("TEST NOTE 20 : stupid_buf_n = %i \n",stupid_buf_n);

    //glueing colors that are the same in RGB555 colorspace
    for (i = 0; i < stupid_buf_n; i++) {
        for (j = i+1; j < stupid_buf_n; j++) {
            if ( (round(stupid_buf[i]->r/(stupid_buf[i]->count*8)) == round(stupid_buf[j]->r/(stupid_buf[j]->count*8))) &&
                   (round(stupid_buf[i]->g/(stupid_buf[i]->count*8)) == round(stupid_buf[j]->g/(stupid_buf[j]->count*8))) &&
                     (round(stupid_buf[i]->b/(stupid_buf[i]->count*8)) == round(stupid_buf[j]->b/(stupid_buf[j]->count*8))) )
            {
                //if (stupid_buf[i]->count > stupid_buf[j]->count)
                {
                    //removing node j
                    for (int k=j+1; k< stupid_buf_n; k++)
                        stupid_buf[k-1] = stupid_buf[k];
                    stupid_buf_n--;
                    i=0;
                    j=1;
                }
            }
        }
    }

    printf("TEST NOTE 30 : stupid_buf_n = %i \n",stupid_buf_n);

    //normalizing and translating the remainder into RGB555 colorspace
    for (i = 0; i < stupid_buf_n; i++) {
        stupid_buf[i]->r = 8*round(stupid_buf[i]->r/(stupid_buf[i]->count*8));
        stupid_buf[i]->g = 8*round(stupid_buf[i]->g/(stupid_buf[i]->count*8));
        stupid_buf[i]->b = 8*round(stupid_buf[i]->b/(stupid_buf[i]->count*8));
    }

    //dropping colors that are not used by image anymore
    uint8_t *usagebuf = malloc(stupid_buf_n+10);
    memset(usagebuf,0,stupid_buf_n+10);
    for (i = 0, pix = im->pix; i < im->w * im->h; i++, pix += 4)
    {
         int _tmp = color_find_stupid(stupid_buf,stupid_buf_n,pix);
         usagebuf[_tmp] = 1;
    }
    for (i=0;i<stupid_buf_n;i++)
    {
        if (usagebuf[i] == 0)
            stupid_buf[i]->count = 0;
    }
    for (j = 0; j < stupid_buf_n; j++) {
        if (stupid_buf[j]->count <= 0)
        {
            //removing node j
                    for (int k=j; k< stupid_buf_n; k++)
                        stupid_buf[k] = stupid_buf[k+1];
                    stupid_buf_n--;
                    j=0;
        }
    }
    free (usagebuf);

    printf("TEST NOTE 40 : stupid_buf_n = %i \n",stupid_buf_n);

    //decreasing color count until requested
    while (stupid_buf_n > n_colors + 1)
    {        
        //searching the lowest diff
        int diff = 100500000;
        int ind1 = -1;
        int ind2 = -1;
        int currdiff = 0;
        for (i = 1; i < stupid_buf_n; i++) {
            for (j = i+1; j < stupid_buf_n; j++) {
                currdiff = (stupid_buf[i]->r - stupid_buf[j]->r)*(stupid_buf[i]->r - stupid_buf[j]->r) +
                        (stupid_buf[i]->g - stupid_buf[j]->g)*(stupid_buf[i]->g - stupid_buf[j]->g) +
                        (stupid_buf[i]->b - stupid_buf[j]->b)*(stupid_buf[i]->b - stupid_buf[j]->b);
                if ( currdiff < diff )
                {
                    ind1 = i;
                    ind2 = j;
                    diff = currdiff;
                }
            }
        }

        if (ind1 < 0) abort();

        //merging colors into ind1
        stupid_buf[ind1]->r = (stupid_buf[ind1]->r*stupid_buf[ind1]->count + stupid_buf[ind2]->r*stupid_buf[ind2]->count) / (stupid_buf[ind1]->count + stupid_buf[ind2]->count);
        stupid_buf[ind1]->g = (stupid_buf[ind1]->g*stupid_buf[ind1]->count + stupid_buf[ind2]->g*stupid_buf[ind2]->count) / (stupid_buf[ind1]->count + stupid_buf[ind2]->count);
        stupid_buf[ind1]->b = (stupid_buf[ind1]->b*stupid_buf[ind1]->count + stupid_buf[ind2]->b*stupid_buf[ind2]->count) / (stupid_buf[ind1]->count + stupid_buf[ind2]->count);

        //translating the ind1 into RGB555 colorspace again
        for (i = 0; i < stupid_buf_n; i++) {
            stupid_buf[ind1]->r = 8*round(stupid_buf[ind1]->r/8);
            stupid_buf[ind1]->g = 8*round(stupid_buf[ind1]->g/8);
            stupid_buf[ind1]->b = 8*round(stupid_buf[ind1]->b/8);
        }

        //removing ind2
        for (int k=ind2+1; k< stupid_buf_n; k++)
            stupid_buf[k-1] = stupid_buf[k];
        stupid_buf_n--;

        printf("TEST NOTE 50 : stupid_buf_n = %i \n",stupid_buf_n);

        //dropping colors that are not used by image anymore
        uint8_t *usagebuf = malloc(stupid_buf_n+10);
        memset(usagebuf,0,stupid_buf_n+10);
        for (i = 0, pix = im->pix; i < im->w * im->h; i++, pix += 4)
        {
             int _tmp = color_find_stupid(stupid_buf,stupid_buf_n,pix);
             usagebuf[_tmp] = 1;
        }
        for (i=0;i<stupid_buf_n;i++)
        {
            if (usagebuf[i] == 0)
                stupid_buf[i]->count = 0;
        }
        for (j = 0; j < stupid_buf_n; j++) {
            if (stupid_buf[j]->count <= 0)
            {
                //removing node j
                        for (int k=j; k< stupid_buf_n; k++)
                            stupid_buf[k] = stupid_buf[k+1];
                        stupid_buf_n--;
                        j=0;
            }
        }
        free (usagebuf);

        printf("TEST NOTE 60 : stupid_buf_n = %i \n",stupid_buf_n);

    }

    //dumping colors
    FILE *colordump_file = fopen("colors.dump","w");
    //debug_dump_colors_recursive(colordump_file, root);
    for (i = 1; i < stupid_buf_n; i++) {
        /*sprintf_s(buf,256,"%i %i %i\n",heap.buf[i]->r/heap.buf[i]->count,
                heap.buf[i]->g/heap.buf[i]->count,
                heap.buf[i]->b/heap.buf[i]->count);*/
        sprintf_s(buf,256,"%i %i %i\n",stupid_buf[i]->r,
                         stupid_buf[i]->g,
                         stupid_buf[i]->b);
        fwrite(buf,strlen(buf),1,colordump_file);
    }
    fclose(colordump_file);


    //int colors = calculate_unique_colors(&heap);
    //printf("Start colors number: %i \n",colors);
    /*while (heap.n > n_colors + 1)
    {
        heap_add(&heap, node_fold(pop_heap(&heap)));
    }*/
    //printf("End colors number: %i \n",colors);

    //round colors
    /*double c;
    for (i = 1; i < heap.n; i++) {
        got = heap.buf[i];
        c = got->count;
        got->r = got->r / c + .5;
        got->g = got->g / c + .5;
        got->b = got->b / c + .5;
    }*/

    /*double c;
    for (i = 1; i < heap.n; i++) {
        got = heap.buf[i];
        c = got->count;
        got->r = got->r / c + .5;
        got->g = got->g / c + .5;
        got->b = got->b / c + .5;

        //got->r = round((got->r / c )/8)*8;
        //got->g = round((got->g / c )/8)*8;
        //got->b = round((got->b / c )/8)*8;
    }*/

    if (dither)
    {
        current_colors = error_diffuse(im, &heap);
    }
    else
    {
        colors_num = 0;
        for (i = 0, pix = im->pix; i < im->w * im->h; i++, pix += 4)
        {
            //color_replace(root, pix);
            color_replace_stupid(stupid_buf,stupid_buf_n,pix);
            //colors calc
            /*color = 0x10000 * pix[0] + 0x100 * pix[1] + pix[2];
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
            }*/
        }
        current_colors = stupid_buf_n;//colors_num;
    }

    free(stupid_buf);
    node_free();
    free(heap.buf);

    return current_colors;
}
