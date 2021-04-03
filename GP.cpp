#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>

// Open CV imports
//#include <core.hpp>
//#include <highgui.hpp>
#include "opencv2/opencv.hpp"
// Open CV imports end

using namespace std;
using namespace cv;

struct dataset {
    string image_path;
    Mat img;
    string label;
};

void readDataset(vector<dataset> * data);

int main() {
    cout << "Hello world" << endl;
    vector<dataset> * data;
    readDataset(data);
}

void readDataset(vector<dataset> * data) {
    //Mat img = imread("./data/1/ped_examples/img_00000.pgm", IMREAD_COLOR);
    Mat img = imread("./test.jpg", IMREAD_COLOR);
    if(img.empty()) {
        std::cout << "No Image to read" << endl;
        return;
    }
    imshow("Display", img);
    int k = waitKey(0);
    if(k=='q') {
        exit(1);
    }
}

namespace GP {
}
