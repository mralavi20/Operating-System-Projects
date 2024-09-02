#include "bmp.h"

using namespace std;


bool fillAndAllocate(char*& buffer, const char* fileName, int& rows, int& cols, int& bufferSize) {
    std::ifstream file(fileName);
    if (!file) {
        std::cout << "File" << fileName << " doesn't exist!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return true;
}

void* getPixelsFromBMP24(void* args) {
    Data* data = (Data *) args;
    char* file_buffer = data->file_buffer;
    int count = data->count;
    int extra = data->cols % 4;
    unsigned char value;
    for (int i = data->start_row; i < data->start_row + data->rows / THREADS_COUNT; i++) {
        count += extra;
        for (int j = data->cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    value = file_buffer[data->end - count];
                    (*data->image)[i][j].red = int (value);
                    break;
                case 1:
                    value = file_buffer[data->end - count];
                    (*data->image)[i][j].green = int (value);
                    break;
                case 2:
                    value = file_buffer[data->end - count];
                    (*data->image)[i][j].blue = int (value);
                    break;
                }
                count = count + 1;
            }
        }
    }
}


void get_pixels (int end, int rows, int cols, char* file_buffer, vector<vector<Pixel>> &image) {
    pthread_t threads[THREADS_COUNT];
    auto data = new Data[THREADS_COUNT];
    int extra;
    int i;

    extra = cols % 4;

    for (i = 0; i < THREADS_COUNT; i++) {
        data[i].rows = rows;
        data[i].cols = cols;
        data[i].end = end;
        data[i].file_buffer = file_buffer;
        data[i].start_row = i * (rows / THREADS_COUNT);
        data[i].count = data[i].start_row * (extra + 3 * cols) + 1;
        data[i].image = &image;

        pthread_create (&threads[i], NULL, &getPixelsFromBMP24, (void *) &data[i]);
    }

    for (i = 0; i < THREADS_COUNT; i++) {
        pthread_join (threads[i], NULL);
    }

    delete (data);
}

void writeOutBmp24(char* fileBuffer, const char* nameOfFileToCreate, int bufferSize, int rows, int cols, vector<vector<Pixel>> &image) {
    std::ofstream write(nameOfFileToCreate);
    if (!write) {
        std::cout << "Failed to write " << nameOfFileToCreate << std::endl;
        return;
    }

    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    fileBuffer[bufferSize - count] = char (image[i][j].red);
                    break;
                case 1:
                    fileBuffer[bufferSize - count] = char (image[i][j].green);
                    break;
                case 2:
                    fileBuffer[bufferSize - count] = char (image[i][j].blue);
                    break;
                }
                count = count + 1;
            }
        }
    }
    write.write(fileBuffer, bufferSize);
}