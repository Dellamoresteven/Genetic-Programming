#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <ctime>
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

    float replaceFeature(int featureToReplace) {
        switch(featureToReplace) {
            case 0:
                return ABED;
            case 1:
                return BCFE;
            case 2:
                return DEHG;
            case 3:
                return EFIH;
            case 4:
                return GHKJ;
            case 5:
                return HILK;
            case 6:
                return JKNM;
            case 7:
                return KLON;
            case 8:
                return PQSR;
            case 9:
                return RSUT;
            case 10:
                return TUWV;
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
    //readFunc("./data/2/ped_examples/", "ped");
    //readFunc("./data/2/non-ped_examples/", "non-ped");
    //readFunc("./data/3/ped_examples/", "ped");
    //readFunc("./data/3/non-ped_examples/", "non-ped");
}

namespace GP {
    // Params for evolution
    int   populationSize = 100;  // Number of agents per round
    int   selectionSize  = 7;    // Number of agents in the tournament selection
    int   maxDepth       = 8;    // Max Depth of classification tree
    int   maxGenerations = 5000; // Max number of generations
    int   numBreed       = 3;    // Number of new agents during breeding
    float mutationRate   = .05;  // % mutation rate of DNA
    float OpChance       = 0.40; // Chance of adding an operator to the DNA
    float constChance    = 0.15; // Chance of adding a constant to the DNA
    float featureChance  = 0.45; // Chance of adding a feature to the DNA
    int   minConst       = -100; // Min const
    int   maxConst       = 100;  // Max const

    vector<Agent*> agents;

    void initPopulation() {
        for(int i = 0; i < populationSize; i++) {
            Agent * newAgent = new Agent();
            newAgent->setRandomDNAStrain([&](bool isRoot) -> gene* {
                if(isRoot) return new gene(true, static_cast<op>(rand()%5), 0);
                float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                if(randNum < OpChance) {
                    return new gene(false, static_cast<op>(rand()%5), 0);
                } else if(randNum < (OpChance + constChance)) {
                    return new gene(false, op::Constant, rand()%20);
                } else {
                    return new gene(false, op::Feature, rand()%11);
                }
            });
            agents.push_back(newAgent);
        }
    }

    void classifyAgents(vector<dataset> data) {
        for(auto & d : data) {
            auto featureTranslator = [&](int featureNum) -> float {
                switch(featureNum) {
                    case 0:
                        return d.ABED;
                    case 1:
                        return d.BCFE;
                    case 2:
                        return d.DEHG;
                    case 3:
                        return d.EFIH;
                    case 4:
                        return d.GHKJ;
                    case 5:
                        return d.HILK;
                    case 6:
                        return d.JKNM;
                    case 7:
                        return d.KLON;
                    case 8:
                        return d.PQSR;
                    case 9:
                        return d.RSUT;
                    case 10:
                        return d.TUWV;
                }
                std::cout << "SOMETHING BROKE HERE1:" << featureNum << std::endl;
                exit(1);
            };
            for(auto & a : agents) {
                a->classification(featureTranslator);
            }
        }
    }

    void fitnessAgents(vector<dataset> data) {
        vector<string> answers;
        for(auto & d : data) {
            answers.push_back(d.label);
        }
        auto fitnessTest = [&](int index, int ans){
            if(ans >= 0 && answers.at(index) == "ped") {
                return true;
            }
            if(ans < 0 && answers.at(index) == "non-ped") {
                return true;
            };
            return false;
        };

        for(auto & a : agents) {
            a->calcSimpleFitness(fitnessTest);
        }
    }

    void resetAgents() {
        for(auto & a : agents) {
            a->reset();
        }
    }

    Agent* crossover(Agent * a1, Agent * a2) {
        Agent * newAgent = new Agent();
        DNA * newDNA = new DNA();
        if(rand()%10 > 5) {
            copyGeneTree(a1->dna->gRoot, newDNA->gRoot, mutationRate);
            copyGeneTree(a2->dna->gRoot->l, newDNA->gRoot->l, mutationRate);
        } else {
            copyGeneTree(a2->dna->gRoot, newDNA->gRoot, mutationRate);
            copyGeneTree(a1->dna->gRoot->l, newDNA->gRoot->l, mutationRate);
        }
        newAgent->dna = newDNA;
        return newAgent;
    }

    void Breed() {
        vector<Agent*> randomAgents;
        for(int i = 0; i < 1; i++) {
            for(int j = 0; j < selectionSize; j++) {
                randomAgents.push_back(agents.at(rand() % populationSize));
            }
            std::sort(randomAgents.begin(), randomAgents.end(), [](Agent* one, Agent* two){
                return one->fitness > two->fitness;
            });
            agents.at(agents.size() - 1) = crossover(randomAgents[0], randomAgents[1]);
            randomAgents.clear();
        }
    }
}


int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    vector<dataset> data;
    readDataset(&data);
    extractFeatures(&data);

    GP::initPopulation();

    for(int k = 0; k < GP::maxGenerations; k++) {
        cout << "GENERATION: " << k << endl;
        // Start of GP stuff
        GP::classifyAgents(data);
        GP::fitnessAgents(data);

        std::sort(GP::agents.begin(), GP::agents.end(), [](Agent* one, Agent* two){
            return one->fitness > two->fitness;
        });
        for(int i = 0; i < 10; i++) {
            cout << "Fitness: " << GP::agents.at(i)->fitness << endl;
        }
        cout << GP::agents.at(0)->dna << endl;
        GP::Breed();
        GP::resetAgents();
    }
}
