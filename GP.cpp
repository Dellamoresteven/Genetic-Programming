#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include "opencv2/opencv.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using namespace cv;
namespace fs = std::filesystem;

struct dataset {
    string image_path;
    Mat img;
    string label;

    // ROI's in the research paper
    float ABED;
    float BCFE;
    float DEHG;
    float EFIH;
    float GHKJ;
    float HILK;
    float JKNM;
    float KLON;
    float PQSR;
    float RSUT;
    float TUWV;
};

void extractFeatures(vector<dataset> * data) {
    // All images should be the same size
    int rows = data->at(0).img.rows;
    int cols = data->at(0).img.cols;

    auto MeanROICalc = [](Mat i, int x1, int x2, int y1, int y2) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(i(cv::Range(x1, x2), cv::Range(y1, y2)), mean, stddev);
        return mean[0];
    };

    for(auto & entry : *data) {
        // Set up ROI's in the research paper
        entry.ABED = MeanROICalc(entry.img,0,rows/4,0,cols/2);
        entry.BCFE = MeanROICalc(entry.img,0,rows/4,cols/2, cols);
        entry.DEHG = MeanROICalc(entry.img,rows/4,rows/2,0,cols/2);
        entry.EFIH = MeanROICalc(entry.img,rows/4, rows/2,cols/2, cols);
        entry.GHKJ = MeanROICalc(entry.img,rows/2, 3*rows/4,0, cols/2);
        entry.HILK = MeanROICalc(entry.img,rows/2, 3*rows/4,cols/2, cols);
        entry.JKNM = MeanROICalc(entry.img,3*rows/4, rows,0, cols/2);
        entry.KLON = MeanROICalc(entry.img,3*rows/4, rows,cols/2, cols);
        entry.PQSR = MeanROICalc(entry.img,0, rows/4,cols/3, 2*cols/3);
        entry.RSUT = MeanROICalc(entry.img,rows/4, (3*rows/4)-2,cols/3, 2*cols/3);
        entry.TUWV = MeanROICalc(entry.img,(3*rows/4)-2, rows-2,cols/3, 2*cols/3);
    }
}

void readDataset(vector<dataset> * data) {
    auto readFunc = [&](string dir, string label) {
        for(const auto & entry : fs::directory_iterator(dir)) {
            dataset d;
            d.img = imread(entry.path(), IMREAD_GRAYSCALE);
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
    // Params for evolution
    int   populationSize = 500;  // Number of agents per round
    int   selectionSize  = 7;    // ?
    int   maxDepth       = 8;    // Max Depth of classification tree
    int   maxGenerations = 50;   // Max number of generations
    float OpChance       = 0.33; // Chance of adding an operator to the DNA
    float constChance    = 0.33; // Chance of adding a constant to the DNA
    float featureChance  = 0.33; // Chance of adding a feature to the DNA
    int   minConst       = -100; // Min const
    int   maxConst       = 100;  // Max const

    struct Agent {
        struct nucleotide {
            int type;
            float value;
        };
        vector<nucleotide> DNA;
        int classification(dataset img) {return -1;}
        int fitness() {return -1;}
        int mutation() {return -1;}
    };

    vector<Agent> agents;

    float randomDNAChoice(int * type, bool isAtMax) {
        float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        if(!isAtMax) {
            if(randNum <= OpChance) {
                *type = 0;
                return float(rand() % 4);
            } else if(randNum <= (OpChance + constChance)) {
                *type = 1;
                return minConst + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxConst - minConst)));
            } else {
                *type = 2;
                return float(rand() % 11);
            }
        } else {
            if(randNum <= (OpChance/2 + constChance)) {
                *type = 1;
                return minConst + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxConst - minConst)));
            } else {
                *type = 2;
                return float(rand() % 11);
            }
        }
    }

    void buildDNA(int op, int depth, auto * DNA) {
        depth += 1;
        int type;
        float first = randomDNAChoice(&type, depth == maxDepth);
        DNA->push_back(Agent::nucleotide(type, first));
        if(type == 0) {
            buildDNA(int(first), depth, DNA);
        }
        float second = randomDNAChoice(&type, depth == maxDepth);
        DNA->push_back(Agent::nucleotide(type, second));
        if(type == 0) {
            buildDNA(int(second), depth, DNA);
        }
    }

    void randomDNA(auto * DNA) {
        DNA->push_back(Agent::nucleotide(0, 0));
        buildDNA(0, 0, DNA);
        for(const auto & d : *DNA) {
            if(d.type == 0) {
                cout << "O";
            } else if(d.type == 2) {
                cout << "F";
            } else {
                cout << "C";
            }
        }
        cout << "\n";
    }

    void initPopulation() {
        for(int i = 0; i < populationSize; i++) {
            Agent a;
            randomDNA(&a.DNA);
            agents.push_back(a);
        }
    }

    void classifyAgents(vector<dataset> data) {
        for(const auto & d : data) {
            for(auto & a : agents) {
                a.classification(d);
            }
        }
    }
}


int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    vector<dataset> data;
    readDataset(&data);
    extractFeatures(&data);

    // Start of GP stuff
    GP::initPopulation();
    GP::classifyAgents(data);
}
