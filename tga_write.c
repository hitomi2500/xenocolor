#include "tga_write.h"

//void tga_write(const char *filename, uint32_t width, uint32_t height, uint8_t *dataBGRA, uint8_t dataChannels=4, uint8_t fileChannels=3)
void tga_write(const char *filename, uint32_t width, uint32_t height, uint8_t *dataBGRA, uint8_t dataChannels, uint8_t fileChannels)
{
    FILE *fp = NULL;
    // MSVC prefers fopen_s, but it's not portable
    fp = fopen(filename, "wb");
    //fopen_s(&fp, filename, "wb");
    if (fp == NULL) return;

    // You can find details about TGA headers here: http://www.paulbourke.net/dataformats/tga/
    uint8_t header[18] = { 0,0,2,0,0,0,0,0,0,0,0,0, (uint8_t)(width%256), (uint8_t)(width/256), (uint8_t)(height%256), (uint8_t)(height/256), (uint8_t)(fileChannels*8), 0x20 };
    fwrite(&header, 18, 1, fp);

    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++)
    {
        for (uint32_t b = 0; b < fileChannels; b++)
        {
            int index = (((height-y-1)*width+x)*dataChannels) + (fileChannels - 1 - b%fileChannels);
            if (index >= width*height*4)
                index = width*height*4-1;
            int value = dataBGRA[index];
            fputc(value, fp);
        }
    }
    fclose(fp);
}
