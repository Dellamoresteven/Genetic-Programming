#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <cstdlib>
#include <iostream>
#include <future>
#include <unistd.h>
#include <vector>
#include "opencv2/opencv.hpp"
#include "DNA.cpp"

#define ASYNC_CODE true

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

    // FEATURES
    vector<float> features;

    float replaceFeature(int featureToReplace) {
        if(featureToReplace >= (int)features.size()) {
            cout << "TOO BIGGGG:" << featureToReplace << " / " << features.size() << endl;
            exit(1);
        }
        return features.at(featureToReplace);
    }
};

void extractFeatures(vector<dataset>& data) {
    // All images should be the same size
    int rows = data.at(0).img.rows; // 36
    int cols = data.at(0).img.cols; // 18

    auto MeanROICalc = [](Mat i, int x1, int x2, int y1, int y2, float& m, float& s) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(i(cv::Range(x1, x2), cv::Range(y1, y2)), mean, stddev);
        m = mean[0];
        s = stddev[0];
    };

    float m, s;
    for(auto & entry : data) {
        for(int i = 0; i < rows; i+=6) {
            for(int j = 0; j < cols; j+=3) {
                MeanROICalc(entry.img,i,i+6,j,j+3, m, s);
                entry.features.push_back(m);
                entry.features.push_back(s);
            }
        }
    }
}

void readDataset(vector<dataset>& data, bool isTraining) {
    auto readFunc = [&](string dir, string label) {
        for(const auto & entry : fs::directory_iterator(dir)) {
            dataset d;
            d.img = imread((string) entry.path(), IMREAD_GRAYSCALE);
            d.image_path = entry.path();
            d.label = label;
            data.push_back(d);
        }
    };
    if(isTraining) {
        readFunc("./data/1/ped_examples/", "ped");
        readFunc("./data/1/non-ped_examples/", "non-ped");
        //readFunc("./data/2/ped_examples/", "ped");
        //readFunc("./data/2/non-ped_examples/", "non-ped");
    } else {
        readFunc("./data/2/ped_examples/", "ped");
        readFunc("./data/2/non-ped_examples/", "non-ped");
    }
}

namespace GP {
    // Params for evolution
    int   populationSize = 500;  // Number of agents per round
    int   selectionSize  = 20;    // Number of agents in the tournament selection
    int   maxDepth       = 8;    // Max Depth of classification tree
    int   maxGenerations = 5000; // Max number of generations
    int   numBreed       = populationSize*.10;    // Number of new agents during breeding
    int   numAdopt       = 10;    // Number of new agents to adopt
    float mutationRate   = .30;  // % mutation rate of DNA
    float OpChance       = 0.4; // Chance of adding an operator to the DNA
    float constChance    = 0.2; // Chance of adding a constant to the DNA
    float featureChance  = 0.4; // Chance of adding a feature to the DNA
    int   maxConst       = 10;  // Max const
    int   numFeatures    = 72; // Number of features

    vector<std::unique_ptr<Agent>> agents;

    void initPopulation(int num) {
        for(int i = 0; i < num; i++) {
            agents.push_back(std::make_unique<Agent>(Agent()));
            agents.at(agents.size() - 1)->setRandomDNAStrain(0, [&](bool isRoot, int currDepth) -> gene* {
                if(isRoot) return new gene(true, static_cast<op>(rand()%5), 0);
                float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                if(randNum < OpChance && currDepth < maxDepth) {
                    return new gene(false, static_cast<op>(rand()%5), 0);
                } else if(randNum < (OpChance + constChance)) {
                    return new gene(false, op::Constant, rand()%maxConst);
                } else {
                    return new gene(false, op::Feature, rand()%numFeatures);
                }
            });
        }
    }

    void classifyAgents(vector<dataset>& data) {
#if !ASYNC_CODE
        for(auto &a : agents) {
            for(auto &d : data) {
                a->classification([&](int index){
                        return d.replaceFeature(index);
                });
            }
        }
#else
        std::vector< std::future<void> > voidFutures;
        for(auto &a : agents) {
            voidFutures.push_back(std::async(std::launch::async,
                [&](){
                    for(auto &d : data) {
                        a->classification([&](int index){
                            return d.replaceFeature(index);
                        });
                    }
                }
            ));
        }
#endif
    }

    float fitnessAgentsAgent(vector<dataset>& data, std::unique_ptr<Agent>& a) {
        vector<string> answers;
        for(auto &d : data) {
            answers.push_back(d.label);
        }

        auto fitnessTest = [&](int index, int ans) {
            if(ans >= 0 && answers.at(index) == "ped") {
                return true;
            }
            else if(ans < 0 && answers.at(index) == "non-ped") {
                return true;
            }
            return false;
        };
        a->calcSimpleFitness(fitnessTest);
        return a->fitness;
    }

    void fitnessAgentsAverage(vector<dataset>& data) {
        vector<string> answers;
        int mp = 0;
        int mn = 0;
        for(auto &d : data) {
            answers.push_back(d.label);
            if(d.label == "ped") {
                mp += 1;
            } else {
                mn += 1;
            }
        }
        auto fitnessTestAvg = [&](int index, int ans, int &pTP, int &pTN, int &pFN, int &pFP) {
            if(ans >= 0 && answers.at(index) == "ped") {
                pTP += 1;
            }
            else if(ans < 0 && answers.at(index) == "non-ped") {
                pTN += 1;
            }
            else if(ans >= 0 && answers.at(index) != "ped") {
                pFP += 1;
            } else if(ans < 0 && answers.at(index) != "non-ped") {
                pFN += 1;
            }
        };
#if !ASYNC_CODE
        for(auto &a : agents) {
            a->calcAverageFitness(fitnessTestAvg, mp, mn);
        }
#else
        std::vector< std::future<void> > voidFutures;
        for(auto &a : agents) {
            voidFutures.push_back(std::async(std::launch::async,
                [&](){
                    a->calcAverageFitness(fitnessTestAvg, mp, mn);
                }
            ));
        }
#endif

    }

    void resetAgents() {
        for(auto & a : agents) {
            a->reset();
        }
    }

    std::unique_ptr<gene> mutFuncTwo(gene* g, const int gDepth) {
        std::unique_ptr<Agent> a = std::make_unique<Agent>();
        a->setRandomDNAStrain(gDepth, [&](bool isRoot, int currDepth) -> gene* {
            float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if(randNum < OpChance && currDepth < maxDepth) {
                return new gene(false, static_cast<op>(rand()%5), 0);
            } else if(randNum < (OpChance + constChance)) {
                return new gene(false, op::Constant, rand()%maxConst);
            } else {
                return new gene(false, op::Feature, rand()%numFeatures);
            }
        });
        gene * ret = a->dna->gRoot.release();
        return std::unique_ptr<gene>(ret);
    }

    Agent crossover(std::unique_ptr<Agent>::pointer a1, std::unique_ptr<Agent>::pointer a2) {
        Agent newAgent = Agent();
        DNA * newDNA = new DNA();
        if(rand()%10 > 5) {
            copyGeneTree(a1->dna->gRoot, newDNA->gRoot, mutFuncTwo, 0, mutationRate);
            copyGeneTree(a2->dna->gRoot->l, newDNA->gRoot->l, mutFuncTwo, 1, mutationRate);
        } else {
            copyGeneTree(a2->dna->gRoot, newDNA->gRoot, mutFuncTwo, 0, mutationRate);
            copyGeneTree(a1->dna->gRoot->l, newDNA->gRoot->l, mutFuncTwo, 1, mutationRate);
        }
        newAgent.dna.reset(newDNA);
        //cout << "NEW AGENT:" << endl;
        //cout << newAgent.dna << endl;
        return newAgent;
    }

    void Breed() {
        vector<std::unique_ptr<Agent>::pointer> randomAgents;
        agents.resize(agents.size()-(numBreed + numAdopt));
        for(int i = 0; i < numBreed; i++) {
            for(int j = 0; j < selectionSize; j++) {
                randomAgents.push_back(agents.at(rand() % agents.size()).get());
            }
            std::sort(randomAgents.begin(), randomAgents.end(), [](std::unique_ptr<Agent>::pointer one, std::unique_ptr<Agent>::pointer two){
                return one->fitness > two->fitness;
            });
            agents.push_back(std::make_unique<Agent>(crossover(randomAgents[0], randomAgents[1])));
            randomAgents.clear();
        }
        initPopulation(numAdopt);
    }

    float testBest(std::unique_ptr<Agent>& a) {
        vector<dataset> data;
        readDataset(data, false);
        extractFeatures(data);
        int TP = 0;
        int numPed = 0;
        int TN = 0;
        int numNonPed = 0;
        for(auto &d : data) {
            if(d.label == "ped") {
                numPed += 1;
            } else {
                numNonPed += 1;
            }
            float score = a->classification([&](int index){
                return d.replaceFeature(index);
            });
            PrettyPrint(score);
            if(score >= 0 && d.label == "ped") {
                TP += 1;
            } else if(score < 0 && d.label == "non-ped") {
                TN += 1;
            }
        }
        cout << TP << " / " << numPed << " = " << float(TP)/numPed << endl;
        cout << TN << " / " << numNonPed << " = " << float(TN)/numNonPed << endl;
        return 0.0;
    }
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    vector<dataset> data;
    readDataset(data, true);
    extractFeatures(data);

    GP::initPopulation(GP::populationSize);
    for(int i = 0; i < GP::maxGenerations; i++) {
    //for(int i = 0; i < 2; i++) {
        cout << "###############" << " " << i << " " << "###############" << endl;
        GP::classifyAgents(data);

        GP::fitnessAgentsAverage(data);
        std::sort(GP::agents.begin(), GP::agents.end(), [](std::unique_ptr<Agent>& one, std::unique_ptr<Agent>& two){
            return one->fitness > two->fitness;
        });
        for(int j = 0; j < 5; j++) {
            PrettyPrint(GP::agents.at(j)->getFitness());
            //PrettyPrint(GP::fitnessAgentsAgent(data, GP::agents.at(j)));
        }
        for(int j = 0; j < 5; j++) {
            PrettyPrint(GP::agents.at(GP::agents.size() - j - 1)->getFitness());
        }
        cout << GP::agents.at(0)->dna << endl;

        GP::Breed();
        GP::resetAgents();
    }

    GP::testBest(GP::agents.at(0));
}
