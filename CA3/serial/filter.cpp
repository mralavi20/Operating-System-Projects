#include "filter.h"

using namespace std;


int kernel[3][3] {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};

void vertical_mirror_filter (int rows, int cols, vector<vector<Pixel>> &image) {
    int i;
    int j;
    Pixel p;

    for (i = 0; i < rows / 2; i++) {
        for (j = 0; j < cols; j++) {
            p = image[i][j];
            image[i][j] = image[rows - i - 1][j];
            image[rows - i - 1][j] = p;
        }
    }
}

vector<int> conv (int rows, int cols, int row_index, int col_index, const vector<vector<Pixel>> &image) {
    int i_index;
    int j_index;
    int i;
    int j;
    vector<int> results = {0, 0, 0};

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            i_index = row_index + i - 1;
            j_index = col_index + j - 1;

            if (i_index < 0 || i_index >= rows) {
                i_index = row_index;
            }
            if (j_index < 0 || j_index >= cols) {
                j_index = col_index;
            }

            results[0] = results[0] + (image[i_index][j_index].red * kernel[i][j]);
            results[1] = results[1] + (image[i_index][j_index].green * kernel[i][j]);
            results[2] = results[2] + (image[i_index][j_index].blue * kernel[i][j]);
        }
    }

    results[0] = int (0.0625 * float (results[0]));
    results[1] = int (0.0625 * float (results[1]));
    results[2] = int (0.0625 * float (results[2]));

    return results;
}

vector<vector<Pixel>> kernel_filter (int rows, int cols, const vector<vector<Pixel>> &image) {
    int i;
    int j;
    vector<int> results;
    vector<vector<Pixel>> filtered_image(rows, vector<Pixel>(cols));

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            results = conv (rows, cols, i, j, image);

            results[0] = max (0, min (255, int (results[0])));

            results[1] = max (0, min (255, int (results[1])));

            results[2] = max (0, min (255, int (results[2])));

            filtered_image[i][j].red = results[0];
            filtered_image[i][j].green = results[1];
            filtered_image[i][j].blue = results[2];

            results.clear ();
        }
    }

    return filtered_image;
}

void purple_haze_filter (int rows, int cols, vector<vector<Pixel>> &image) {
    int red;
    int green;
    int blue;
    int i;
    int j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            red = image[i][j].red;
            green = image[i][j].green;
            blue = image[i][j].blue;

            image[i][j].red = 0.5 * red + 0.3 * green + 0.5 * blue;
            image[i][j].green = 0.16 * red + 0.5 * green + 0.16 * blue;
            image[i][j].blue = 0.6 * red + 0.2 * green + 0.8 * blue;

            image[i][j].red = max (BLACK, image[i][j].red);
            image[i][j].red = min (WHITE, image[i][j].red);

            image[i][j].green = max (BLACK, image[i][j].green);
            image[i][j].green = min (WHITE, image[i][j].green);

            image[i][j].blue = max (BLACK, image[i][j].blue);
            image[i][j].blue = min (WHITE, image[i][j].blue);
        }
    }
}

void add_lines (int rows, int cols, vector<vector<Pixel>> &image) {
    int i;
    int j;
    
    for (i = 0; i < rows; i++) {
        j = cols - i - 1;

        image[i][j].red = WHITE;
        image[i][j].green = WHITE;
        image[i][j].blue = WHITE;
    }

    for (i = 0; i < rows / 2; i++) {
        j =  cols - (i + cols / 2) - 1;

        image[i][j].red = WHITE;
        image[i][j].green = WHITE;
        image[i][j].blue = WHITE;
    }

    for (j = cols / 2; j < cols; j++) {
        i = rows - 1 - (j - cols / 2);

        image[i][j].red = WHITE;
        image[i][j].green = WHITE;
        image[i][j].blue = WHITE;
    }
}