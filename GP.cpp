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

    // ROI's in the research paper
    Mat ABED;
    Mat BCFE;
    Mat DEHG;
    Mat EFIH;
    Mat GHKJ;
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
    int rows = data->at(0).img.rows;
    int cols = data->at(0).img.cols;
    for(auto & entry : *data) {
        cout << rows << ":" << cols << endl;
        // Set up ROI's in the research paper
        entry.ABED = entry.img(cv::Range(0, rows/4), cv::Range(0, cols/2));
        entry.BCFE = entry.img(cv::Range(0, rows/4), cv::Range(cols/2, cols));
        entry.DEHG = entry.img(cv::Range(rows/4, rows/2), cv::Range(0, cols/2));
        entry.EFIH = entry.img(cv::Range(rows/4, rows/2), cv::Range(cols/2, cols));
        entry.GHKJ = entry.img(cv::Range(rows/2, 3*rows/4), cv::Range(0, cols/2));
        entry.HILK = entry.img(cv::Range(rows/2, 3*rows/4), cv::Range(cols/2, cols));
        entry.JKNM = entry.img(cv::Range(3*rows/4, rows), cv::Range(0, cols/2));
        entry.KLON = entry.img(cv::Range(3*rows/4, rows), cv::Range(cols/2, cols));
        entry.PQSR = entry.img(cv::Range(0, rows/4), cv::Range(cols/3, 2*cols/3));
        entry.RSUT = entry.img(cv::Range(rows/4, (3*rows/4)-2), cv::Range(cols/3, 2*cols/3));
        entry.TUWV = entry.img(cv::Range((3*rows/4)-2, rows-2), cv::Range(cols/3, 2*cols/3));
        //cv::imshow("Full", entry.img);
        //cv::waitKey(0);
    }
}

void readDataset(vector<dataset> * data) {
    auto readFunc = [&](string dir, string label) {
        for(const auto & entry : fs::directory_iterator(dir)) {
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
