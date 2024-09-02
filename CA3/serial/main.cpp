#include <vector>
#include <chrono>
#include "bmp.h"
#include "filter.h"

using namespace std;


int main(int argc, char* argv[]) {
    char* fileBuffer;
    int bufferSize;
    int rows;
    int cols;
    if (!fillAndAllocate(fileBuffer, argv[1], rows, cols, bufferSize)) {
        std::cout << "File read error" << std::endl;
        return 1;
    }

    vector<int> times;
    vector<vector<Pixel>> image(rows, vector<Pixel>(cols));

    auto start_exec = std::chrono::high_resolution_clock::now ();

    auto start1 = std::chrono::high_resolution_clock::now ();
    getPixelsFromBMP24 (bufferSize, rows, cols, fileBuffer, image);
    auto end1 = std::chrono::high_resolution_clock::now ();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count ();
    cout << "Read: " << float (duration1) / 1000 << " ms" << endl;

    auto start2 = std::chrono::high_resolution_clock::now ();
    vertical_mirror_filter (rows, cols, image);
    auto end2 = std::chrono::high_resolution_clock::now ();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count ();
    cout << "Flip: " << float (duration2) / 1000 << " ms" << endl;
    
    auto start3 = std::chrono::high_resolution_clock::now ();
    image = kernel_filter (rows, cols, image);
    auto end3 = std::chrono::high_resolution_clock::now ();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3).count ();
    cout << "Blur: " << float (duration3) / 1000 << " ms" << endl;

    auto start4 = std::chrono::high_resolution_clock::now ();
    purple_haze_filter (rows, cols, image);
    auto end4 = std::chrono::high_resolution_clock::now ();
    auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(end4 - start4).count ();
    cout << "Purple: " << float (duration4) / 1000 << " ms" << endl;

    auto start5 = std::chrono::high_resolution_clock::now ();
    add_lines (rows, cols, image);
    auto end5 = std::chrono::high_resolution_clock::now ();
    auto duration5 = std::chrono::duration_cast<std::chrono::microseconds>(end5 - start5).count ();
    cout << "Lines: " << float (duration5) / 1000 << " ms" << endl;

    writeOutBmp24 (fileBuffer, "image.bmp", bufferSize, rows, cols, image);

    auto end_exec = std::chrono::high_resolution_clock::now ();
    auto execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end_exec - start_exec).count ();

    cout << "Execution: " << float (execution_time) / 1000 << " ms" << endl;

    return 0;
}
