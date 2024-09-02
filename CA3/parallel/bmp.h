#ifndef BMP_H
#define BMP_H

#include <fstream>
#include <iostream>
#include <vector>
#include <pthread.h>

#define THREADS_COUNT 4


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

typedef struct Values {
    int start;
    int rows;
    int cols;
    std::vector<std::vector<Pixel>>* image;
    std::vector<std::vector<Pixel>>* temp_image;
} Values;

typedef struct Data {
    int end;
    int rows;
    int cols;
    int start_row;
    int count;
    char* file_buffer;
    std::vector<std::vector<Pixel>>* image;
} Data;


bool fillAndAllocate(char*& buffer, const char* fileName, int& rows, int& cols, int& bufferSize);
void* getPixelsFromBMP24(void* args);
void get_pixels (int end, int rows, int cols, char* file_buffer, std::vector<std::vector<Pixel>> &image);
void writeOutBmp24(char* fileBuffer, const char* nameOfFileToCreate, int bufferSize, int rows, int cols, std::vector<std::vector<Pixel>> &image);

#endif