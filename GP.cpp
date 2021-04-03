#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <filesystem>

// Open CV imports
//#include <core.hpp>
//#include <highgui.hpp>
#include "opencv2/opencv.hpp"
// Open CV imports end

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

struct dataset {
    string image_path;
    Mat img;
    string label;
    Mat ABED;
    Mat BCFE;
    Mat DEHG;
    Mat EFIH;
    Mat DHKJ;
    Mat HILK;
    Mat JKNM;
    Mat KLON;
    Mat PQSR;
    Mat RSUT;
    Mat TUWV;
};

void readDataset(vector<dataset> * data);
void extractFeatures(vector<dataset> * data);

int main() {
    vector<dataset> data;
    readDataset(&data);
    extractFeatures(&data);
}

void extractFeatures(vector<dataset> * data) {
}

void readDataset(vector<dataset> * data) {
    auto readFunc = [&](string dir, string label) {
        for(const auto & entry: fs::directory_iterator(dir)) {
            dataset d;
            d.img = imread(entry.path(), IMREAD_COLOR);
            d.image_path = entry.path();
            d.label = label;
            data->push_back(d);
        }
    };
    readFunc("./data/1/ped_examples/", "ped");
    readFunc("./data/1/non-ped_examples/", "non-ped");
    readFunc("./data/2/ped_examples/", "ped");
    readFunc("./data/2/non-ped_examples/", "non-ped");
    //readFunc("./data/3/ped_examples/", "ped");
    //readFunc("./data/3/non-ped_examples/", "non-ped");
}

namespace GP {
}
