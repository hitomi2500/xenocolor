#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tga.h"
#include "tga_write.h"
#include "main.h"
#include "quantize.h"

void build_octree_recursive(struct octree_node_type octree_node)
{
    if (octree_node.size <= 5)
        return;

    for (int i=0;i<8;i++)
    {
        octree_node.childs[i] =  malloc(sizeof(struct octree_node_type));
        octree_node.childs[i]->parent = &octree_node;
        octree_node.childs[i]->leafs_number = 0;
        octree_node.childs[i]->size = octree_node.size/2;
    }

    octree_node.childs[0]->r0 = octree_node.r0;
    octree_node.childs[0]->g0 = octree_node.g0;
    octree_node.childs[0]->b0 = octree_node.b0;
    octree_node.childs[1]->r0 = octree_node.r0 + octree_node.size/2;
    octree_node.childs[1]->g0 = octree_node.g0;
    octree_node.childs[1]->b0 = octree_node.b0;
    octree_node.childs[2]->r0 = octree_node.r0;
    octree_node.childs[2]->g0 = octree_node.g0 + octree_node.size/2;
    octree_node.childs[2]->b0 = octree_node.b0;
    octree_node.childs[3]->r0 = octree_node.r0 + octree_node.size/2;
    octree_node.childs[3]->g0 = octree_node.g0 + octree_node.size/2;
    octree_node.childs[3]->b0 = octree_node.b0;
    octree_node.childs[4]->r0 = octree_node.r0;
    octree_node.childs[4]->g0 = octree_node.g0;
    octree_node.childs[4]->b0 = octree_node.b0 + octree_node.size/2;
    octree_node.childs[5]->r0 = octree_node.r0 + octree_node.size/2;
    octree_node.childs[5]->g0 = octree_node.g0;
    octree_node.childs[5]->b0 = octree_node.b0 + octree_node.size/2;
    octree_node.childs[6]->r0 = octree_node.r0;
    octree_node.childs[6]->g0 = octree_node.g0 + octree_node.size/2;
    octree_node.childs[6]->b0 = octree_node.b0 + octree_node.size/2;
    octree_node.childs[7]->r0 = octree_node.r0 + octree_node.size/2;
    octree_node.childs[7]->g0 = octree_node.g0 + octree_node.size/2;
    octree_node.childs[7]->b0 = octree_node.b0 + octree_node.size/2;

    for (int i=0;i<8;i++)
    {
        build_octree_recursive(octree_node.childs[i][0]);
    }

}


void print_usage(char * binaryname)
{
    printf("Usage: ");
    printf((char*)binaryname);
    printf(" [OPERANDS] [FLAGS] INPUTFILE [OUTPUTFILE] \n");
    printf("Supported operands: \n");
    printf("    l=COLORS      limit output image colors number, default 256\n");
    //printf("    b=BPP         set output image bits per pixel, supported values are 1, 2, 4, 8, 16, 24 and 32(default)  \n");
    //printf("    k=KEY         hex color identifier for the color that should be set as index 0 if present (defaults to FF00FF)  \n");
    printf("    d=0/1         dithering enable/disable \n");
    //printf("Supported flags: \n");
    //printf("    v             verbose, output detailed log  \n");
}

int main(int argc, char *argv[] )
{
    printf("xenocolor v 0.2\n");

    //print_usage(argv[0]);

    int color_limit = 256;
    int output_bpp = 32;
    int dithering = 0;
    int colorkey = 0xFF00FF;
    char buf[256];
    char filename[1024];
    char filename_out[1024];
    char filename2[1024];

    strcpy(filename,"in0.tga");
    strcpy(filename_out,"out.tga");

    //parsing input values
    for (int i=1;i<argc;i++)
    {
        if (strstr(argv[i],"l=")  != NULL)
        {
            //l operand
            color_limit = atoi(&(argv[i][2]));
            if ( (color_limit < 2) || (color_limit>256) )
            {
                printf("Colors number is out of range, requested %i, supported 2 to 256\n",color_limit);
                print_usage(argv[0]);
                exit (-1);
            }
        }
        else if (strstr(argv[i],"b=")  != NULL)
        {
            //b operand
            output_bpp = atoi(&(argv[i][2]));
            if ( (output_bpp != 1) && (output_bpp != 2)  && (output_bpp != 4)  && (output_bpp != 8)  && (output_bpp != 16)  && (output_bpp != 24)  && (output_bpp != 32) )
            {
                printf("Bits per pixel is out of range, requested %i, supported 1, 2, 4, 8, 16, 24 and 32\n",output_bpp);
                print_usage(argv[0]);
                exit (-2);
            }
        }
        /*else if (strstr(argv[i],"k=")  != NULL)
        {
            //k operand
            colorkey = atoi(&(argv[i][2]));
            if ( (output_bpp != 1) && (output_bpp != 2)  && (output_bpp != 4)  && (output_bpp != 8)  && (output_bpp != 16)  && (output_bpp != 24)  && (output_bpp != 32) )
            {
                printf("Bits per pixel is out of range, requested %i, supported 1, 2, 4, 8, 16, 24 and 32\n",output_bpp);
                exit (-2);
            }
        }*/
        else if (strstr(argv[i],"d=")  != NULL)
        {
            //b operand
            dithering = atoi(&(argv[i][2]));
            if ( (dithering != 1) && (dithering != 0) )
            {
                printf("Unacceplatble dithering value %i, only 0 and 1 are accepted\n",dithering);
                print_usage(argv[0]);
                exit (-3);
            }
        }
        else
        {
            strcpy(filename,argv[i]);
            if (filename[0] == '\'')
            {
                strcpy(filename2,&(filename[1]));
                char * pc = strchr(filename2,'\'');
                if (pc != NULL)
                {
                    pc[0] = '\0';
                    strcpy(filename,filename2);
                }
            }
            if (i < (argc-1))
            {
                i++;
                strcpy(filename_out,argv[i]);
                if (filename_out[0] == '\'')
                {
                    strcpy(filename2,&(filename_out[1]));
                    char * pc = strchr(filename2,'\'');
                    if (pc != NULL)
                    {
                        pc[0] = '\0';
                        strcpy(filename_out,filename2);
                    }
                }
            }
        }
    }

    FILE *input_file = fopen(filename,"r");
    if (input_file == NULL)
    {
        printf("File %s not found\n",filename);
        print_usage(argv[0]);
        exit (-10);
    }
    fclose(input_file);
    TgaImage *img = TgaImage_new(filename);
    if (img == NULL)
    {
        printf("Unknown TGA format in %s\n",filename);
        exit (-11);
    }
    image_t im;
    im.w = TgaImage_getWidth(img);
    im.h = TgaImage_getHeight(img);
    im.pix = TgaImage_getData(img);
    if (im.w*im.h <=0)
    {
        printf("Unknown TGA format\n");
        exit (-4);
    }
    printf("%s : %i x %i (%i pixels)\n",filename,im.w,im.h,im.w*im.h);

    TgaImage_destroy(img);
    img = NULL;

    int current_colors_limit=color_limit;
    int colors;
    int iteration=1;
    char bbuf[60];
    //do
    {
        if (img != NULL)
        {
            TgaImage_destroy(img);
            img = NULL;
        }
        img = TgaImage_new(filename);
        im.w = TgaImage_getWidth(img);
        im.h = TgaImage_getHeight(img);
        im.pix = TgaImage_getData(img);
        /*if (iteration==3)
            for (int i=0;i<20000;i++) im_tmp.pix[i] = 0x0;*/

        //replacfe alpha with pink
        for (int y=0;y<im.h;y++)
            for (int x=0;x<im.w;x++)
                if (im.pix[(y*im.w+x)*4+3] < 127)
                {
                    im.pix[(y*im.w+x)*4+0] = 255;
                    im.pix[(y*im.w+x)*4+1] = 0;
                    im.pix[(y*im.w+x)*4+2] = 255;
                    im.pix[(y*im.w+x)*4+3] = 255;
                }

        colors = color_quant(&im, current_colors_limit, dithering);
        printf("Colors number : %i ( limit %i), requested %i, diff %i \n",colors,current_colors_limit,color_limit,abs(colors - color_limit));
        if (colors > color_limit)
            current_colors_limit = (current_colors_limit * colors)/color_limit; //increasing
        else
            current_colors_limit = (current_colors_limit * color_limit)/colors; //decreasing

        //sprintf(bbuf,"%s_step%i.tga",filename_out,iteration);
        //tga_write(bbuf,im_tmp.w,im_tmp.h,im_tmp.pix,4,3);
        iteration++;

    } //while ( (abs(colors-color_limit)>10) && (current_colors_limit < 1024) && (iteration<10) );  //there's some degradation after 256, reason unknown atm

    //save tga
    printf("Saving as %s\n",filename_out);
    tga_write(filename_out,im.w,im.h,im.pix,4,3);
    printf("Done\n");

    return 0;
}
