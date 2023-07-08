#pragma once

#define DEFAULT_PPM_X 3780
#define DEFAULT_PPM_Y 3780

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned short signature;
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offsetToPixelArray;
} BitmapFileHeader;

typedef struct
{
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    unsigned int xPixelsPerMeter;
    unsigned int yPixelsPerMeter;
    unsigned int colorsInColorTable;
    unsigned int importantColorCount;
} BimapV3Header;
#pragma pack(pop)
