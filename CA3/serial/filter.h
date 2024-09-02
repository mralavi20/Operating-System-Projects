#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include "bmp.h"

#define WHITE 255
#define BLACK 0

void vertical_mirror_filter (int rows, int cols, std::vector<std::vector<Pixel>> &image);
std::vector<int> conv (int rows, int cols, int row_index, int col_index, const std::vector<std::vector<Pixel>> &image);
std::vector<std::vector<Pixel>> kernel_filter (int rows, int cols, const std::vector<std::vector<Pixel>> &image);
void purple_haze_filter (int rows, int cols, std::vector<std::vector<Pixel>> &image);
void add_lines (int rows, int cols, std::vector<std::vector<Pixel>> &image);

#endif