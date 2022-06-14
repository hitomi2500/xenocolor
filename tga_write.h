#ifndef TGAWRITE_H
#define TGAWRITE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void tga_write(const char *filename, uint32_t width, uint32_t height, uint8_t *dataBGRA, uint8_t dataChannels, uint8_t fileChannels);


#endif
