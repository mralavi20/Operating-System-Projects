#ifndef BMP_H
#define BMP_H

#include <fstream>
#include <iostream>
#include <vector>


typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

typedef struct Pixel {
    int red;
    int green;
    int blue;
} Pixel;

bool fillAndAllocate(char*& buffer, const char* fileName, int& rows, int& cols, int& bufferSize);
void getPixelsFromBMP24(int end, int rows, int cols, char* fileReadBuffer, std::vector<std::vector<Pixel>> &image);
void writeOutBmp24(char* fileBuffer, const char* nameOfFileToCreate, int bufferSize, int rows, int cols, std::vector<std::vector<Pixel>> &image);

#endif