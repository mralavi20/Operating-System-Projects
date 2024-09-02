#include "filter.h"

using namespace std;


int kernel[3][3] {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};

void* vertical_mirror_filter_part (void* args) {
    int i;
    int j;
    Pixel p;
    Values *values = (Values *)args;

    for (i = 0; i < values->rows / 2; i++) {
        for (j = values->start; j < values->start + values->cols / THREADS_COUNT; j++) {
            p = (*values->image)[i][j];
            (*values->image)[i][j] = (*values->image)[values->rows - i - 1][j];
            (*values->image)[values->rows - i - 1][j] = p;
        }
    }
}

void vertical_mirror_filter (int rows, int cols, vector<vector<Pixel>> &image) {
    pthread_t threads[THREADS_COUNT];
    auto values = new Values[THREADS_COUNT];
    int i;

    for (i = 0; i < THREADS_COUNT; i++) {
        values[i].start = i * (cols / THREADS_COUNT);
        values[i].image = &image;
        values[i].rows = rows;
        values[i].cols = cols;

        pthread_create (&threads[i], NULL, &vertical_mirror_filter_part, (void *) &values[i]);
    }

    for (i = 0; i < THREADS_COUNT; i++) {
        pthread_join (threads[i], NULL);
    }

    delete (values);
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

void* kernel_filter_part (void* args) {
    int i;
    int j;
    vector<int> results;
    Values* values = (Values *)args;

    for (i = values->start; i < values->start + values->rows / THREADS_COUNT; i++) {
        for (j = 0; j < values->cols; j++) {
            results = conv (values->rows, values->cols, i, j, *values->image);

            results[0] = max (BLACK, min (WHITE, int (results[0])));

            results[1] = max (BLACK, min (WHITE, int (results[1])));

            results[2] = max (BLACK, min (WHITE, int (results[2])));

            (*values->temp_image)[i][j].red = results[0];
            (*values->temp_image)[i][j].green = results[1];
            (*values->temp_image)[i][j].blue = results[2];

            results.clear ();
        }
    }
}

void kernel_filter (int rows, int cols, vector<vector<Pixel>> &image) {
    pthread_t threads[THREADS_COUNT];
    auto values = new Values[THREADS_COUNT];
    vector<vector<Pixel>> temp_image(rows, vector<Pixel>(cols));
    int i;

    for (i = 0; i < THREADS_COUNT; i++) {
        values[i].start = i * (rows / THREADS_COUNT);
        values[i].image = &image;
        values[i].temp_image = &temp_image;
        values[i].rows = rows;
        values[i].cols = cols;

        pthread_create (&threads[i], NULL, &kernel_filter_part, (void *) &values[i]);
    }

    for (i = 0; i < THREADS_COUNT; i++) {
        pthread_join (threads[i], NULL);
    }

    image = temp_image;

    delete (values);
}

void* purple_haze_filter_part (void* args) {
    Values* values = (Values *) args;
    int red;
    int green;
    int blue;
    int i;
    int j;
    
    for (i = values->start; i < values->start + values->rows / THREADS_COUNT; i++) {
        for (j = 0; j < values->cols; j++) {
            red = (*values->image)[i][j].red;
            green = (*values->image)[i][j].green;
            blue = (*values->image)[i][j].blue;

            (*values->image)[i][j].red = 0.5 * red + 0.3 * green + 0.5 * blue;
            (*values->image)[i][j].green = 0.16 * red + 0.5 * green + 0.16 * blue;
            (*values->image)[i][j].blue = 0.6 * red + 0.2 * green + 0.8 * blue;

            (*values->image)[i][j].red = max (BLACK, (*values->image)[i][j].red);
            (*values->image)[i][j].red = min (WHITE, (*values->image)[i][j].red);

            (*values->image)[i][j].green = max (BLACK, (*values->image)[i][j].green);
            (*values->image)[i][j].green = min (WHITE, (*values->image)[i][j].green);

            (*values->image)[i][j].blue = max (BLACK, (*values->image)[i][j].blue);
            (*values->image)[i][j].blue = min (WHITE, (*values->image)[i][j].blue);
        }
    }
}

void purple_haze_filter (int rows, int cols, vector<vector<Pixel>> &image) {
    pthread_t threads[THREADS_COUNT];
    auto values = new Values[THREADS_COUNT];
    int i;

    for (i = 0; i < THREADS_COUNT; i++) {
        values[i].start = i * (rows / THREADS_COUNT);
        values[i].image = &image;
        values[i].rows = rows;
        values[i].cols = cols;

        pthread_create (&threads[i], NULL, &purple_haze_filter_part, (void *) &values[i]);
    }
    
    for (i = 0; i < THREADS_COUNT; i++) {
        pthread_join (threads[i], NULL);
    }

    delete (values);
}

void* add_first_line (void* args) {
    Values* values = (Values *) args;
    int i;
    int j;

    for (i = values->start; i < values->start + values->rows / 2; i++) {
        j = values->cols - i - 1;

        (*values->image)[i][j].red = WHITE;
        (*values->image)[i][j].green = WHITE;
        (*values->image)[i][j].blue = WHITE;
    }
}

void* add_second_line (void* args) {
    Values* values = (Values *) args;
    int i;
    int j;

    for (i = 0; i < values->rows / 2; i++) {
        j =  values->cols - (i + values->cols / 2) - 1;

        (*values->image)[i][j].red = WHITE;
        (*values->image)[i][j].green = WHITE;
        (*values->image)[i][j].blue = WHITE;
    }
}

void* add_third_line (void* args) {
    Values* values = (Values *) args;
    int i;
    int j;

    for (j = values->cols / 2; j < values->cols; j++) {
        i = values->rows - 1 - (j - values->cols / 2);

        (*values->image)[i][j].red = WHITE;
        (*values->image)[i][j].green = WHITE;
        (*values->image)[i][j].blue = WHITE;
    }
}

void add_lines (int rows, int cols, vector<vector<Pixel>> &image) {
    pthread_t threads[4];
    auto values = new Values[4];
    int i;

    for (i = 0; i < 2; i++) {
        values[i].start = i * (rows / 2);
        values[i].image = &image;
        values[i].rows = rows;
        values[i].cols = cols;

        pthread_create (&threads[i], NULL, &add_first_line, (void *) &values[i]);
    }

    values[2].image = &image;
    values[2].rows = rows;
    values[2].cols = cols;
    pthread_create (&threads[2], NULL, &add_second_line, (void *) &values[2]);

    values[3].image = &image;
    values[3].rows = rows;
    values[3].cols = cols;
    pthread_create (&threads[3], NULL, &add_third_line, (void *) &values[3]);

    for (i = 0; i < 4; i++) {
        pthread_join (threads[i], NULL);
    }

    delete (values);
}