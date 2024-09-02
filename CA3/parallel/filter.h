#ifndef FILTER_H
#define FILTER_H

#include <fstream>
#include <iostream>
#include <vector>
#include <pthread.h>
#include "bmp.h"

#define WHITE 255
#define BLACK 0


void* vertical_mirror_filter_part (void* args);
void vertical_mirror_filter (int rows, int cols, std::vector<std::vector<Pixel>> &image);
std::vector<int> conv (int rows, int cols, int row_index, int col_index, const std::vector<std::vector<Pixel>> &image);
void* kernel_filter_part (void* args);
void kernel_filter (int rows, int cols, std::vector<std::vector<Pixel>> &image);
void* purple_haze_filter_part (void* args);
void purple_haze_filter (int rows, int cols, std::vector<std::vector<Pixel>> &image);
void* add_first_line (void* args);
void* add_second_line (void* args);
void* add_third_line (void* args);
void add_lines (int rows, int cols, std::vector<std::vector<Pixel>> &image);

#endif