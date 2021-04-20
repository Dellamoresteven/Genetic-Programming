#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <limits>
#include "opencv2/opencv.hpp"
#include "DNA.cpp"

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
    float ABEDmean;
    float BCFEmean;
    float DEHGmean;
    float EFIHmean;
    float GHKJmean;
    float HILKmean;
    float JKNMmean;
    float KLONmean;
    float PQSRmean;
    float RSUTmean;
    float TUWVmean;

    float ABEDstd;
    float BCFEstd;
    float DEHGstd;
    float EFIHstd;
    float GHKJstd;
    float HILKstd;
    float JKNMstd;
    float KLONstd;
    float PQSRstd;
    float RSUTstd;
    float TUWVstd;

    float replaceFeature(int featureToReplace) {
        switch(featureToReplace) {
            case 0:
                return ABEDmean;
            case 1:
                return BCFEmean;
            case 2:
                return DEHGmean;
            case 3:
                return EFIHmean;
            case 4:
                return GHKJmean;
            case 5:
                return HILKmean;
            case 6:
                return JKNMmean;
            case 7:
                return KLONmean;
            case 8:
                return PQSRmean;
            case 9:
                return RSUTmean;
            case 10:
                return TUWVmean;
            case 11:
                return ABEDstd;
            case 12:
                return BCFEstd;
            case 13:
                return DEHGstd;
            case 14:
                return EFIHstd;
            case 15:
                return GHKJstd;
            case 16:
                return HILKstd;
            case 17:
                return JKNMstd;
            case 18:
                return KLONstd;
            case 19:
                return PQSRstd;
            case 20:
                return RSUTstd;
            case 21:
                return TUWVstd;
            default:
                cout << "feature to large" << endl;
                exit(1);
        }
    }
};

void extractFeatures(vector<dataset> * data) {
    // All images should be the same size
    int rows = data->at(0).img.rows;
    int cols = data->at(0).img.cols;

    auto MeanROICalc = [](Mat i, int x1, int x2, int y1, int y2, float& m, float& s) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(i(cv::Range(x1, x2), cv::Range(y1, y2)), mean, stddev);
        m = mean[0];
        s = stddev[0];
    };

    for(auto & entry : *data) {
        // Set up ROI's in the research paper
        MeanROICalc(entry.img,0,rows/4,0,cols/2, entry.ABEDmean, entry.ABEDstd);
        MeanROICalc(entry.img,0,rows/4,cols/2, cols, entry.BCFEmean, entry.BCFEstd);
        MeanROICalc(entry.img,rows/4,rows/2,0,cols/2, entry.DEHGmean, entry.DEHGstd);
        MeanROICalc(entry.img,rows/4, rows/2,cols/2, cols, entry.EFIHmean, entry.EFIHstd);
        MeanROICalc(entry.img,rows/2, 3*rows/4,0, cols/2, entry.GHKJmean, entry.GHKJmean);
        MeanROICalc(entry.img,rows/2, 3*rows/4,cols/2, cols, entry.HILKmean, entry.HILKstd);
        MeanROICalc(entry.img,3*rows/4, rows,0, cols/2, entry.JKNMmean, entry.JKNMstd);
        MeanROICalc(entry.img,3*rows/4, rows,cols/2, cols, entry.KLONmean, entry.KLONstd);
        MeanROICalc(entry.img,0, rows/4,cols/3, 2*cols/3, entry.PQSRmean, entry.PQSRstd);
        MeanROICalc(entry.img,rows/4, (3*rows/4)-2,cols/3, 2*cols/3, entry.RSUTmean, entry.RSUTstd);
        MeanROICalc(entry.img,(3*rows/4)-2, rows-2,cols/3, 2*cols/3, entry.TUWVmean, entry.TUWVstd);
    }
}

void readDataset(vector<dataset> * data, bool isTraining) {
    auto readFunc = [&](string dir, string label) {
        for(const auto & entry : fs::directory_iterator(dir)) {
            dataset d;
            d.img = imread((string) entry.path(), IMREAD_GRAYSCALE);
            d.image_path = entry.path();
            d.label = label;
            data->push_back(d);
        }
    };
    if(isTraining) {
        readFunc("./data/1/ped_examples/", "ped");
        readFunc("./data/1/non-ped_examples/", "non-ped");
        //readFunc("./data/2/ped_examples/", "ped");
        //readFunc("./data/2/non-ped_examples/", "non-ped");
    } else {
        readFunc("./data/3/ped_examples/", "ped");
        readFunc("./data/3/non-ped_examples/", "non-ped");
    }
}

namespace GP {
    // Params for evolution
    int   populationSize = 200;  // Number of agents per round
    int   selectionSize  = 10;    // Number of agents in the tournament selection
    int   maxDepth       = 10;    // Max Depth of classification tree
    int   maxGenerations = 100; // Max number of generations
    int   numBreed       = 10;    // Number of new agents during breeding
    int   numAdopt       = 10;    // Number of new agents to adopt
    float mutationRate   = .1;  // % mutation rate of DNA
    float OpChance       = 0.40; // Chance of adding an operator to the DNA
    float constChance    = 0.1; // Chance of adding a constant to the DNA
    float featureChance  = 0.5; // Chance of adding a feature to the DNA
    int   maxConst       = 10;  // Max const
    int   bestFitness    = 0; // Best fitness of all gens

    int orgnumAdopt = numAdopt;
    int orgmutationRate = mutationRate;

    vector<std::unique_ptr<Agent>> agents;

    void initPopulation(int num) {
        for(int i = 0; i < populationSize; i++) {
            agents.push_back(std::make_unique<Agent>(Agent()));
            agents.at(agents.size())->setRandomDNAStrain([&](bool isRoot, int currDepth) -> gene* {
                if(isRoot) return new gene(true, static_cast<op>(rand()%5), 0);
                float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                if(randNum < OpChance && currDepth < maxDepth) {
                    return new gene(false, static_cast<op>(rand()%5), 0);
                } else if(randNum < (OpChance + constChance)) {
                    return new gene(false, op::Constant, rand()%maxConst);
                } else {
                    return new gene(false, op::Feature, rand()%11);
                }
            });
        }
    }

    void classifyAgents(vector<dataset> data) {
    }

    void fitnessAgents(vector<dataset> data) {
    }

    void resetAgents() {
        for(auto & a : agents) {
            a->reset();
        }
    }

    std::unique_ptr<gene> mutFunc(std::unique_ptr<gene> g) {
    }

    Agent* crossover(Agent * a1, Agent * a2) {
    }

    void Breed() {
    }
}


int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    vector<dataset> data;
    readDataset(&data, true);
    extractFeatures(&data);
}
