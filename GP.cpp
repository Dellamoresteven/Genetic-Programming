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

void extractFeatures(vector<dataset>& data) {
    // All images should be the same size
    int rows = data.at(0).img.rows;
    int cols = data.at(0).img.cols;

    auto MeanROICalc = [](Mat i, int x1, int x2, int y1, int y2, float& m, float& s) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(i(cv::Range(x1, x2), cv::Range(y1, y2)), mean, stddev);
        m = mean[0];
        s = stddev[0];
    };

    for(auto & entry : data) {
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
        readFunc("./data/3/ped_examples/", "ped");
        readFunc("./data/3/non-ped_examples/", "non-ped");
    }
}

namespace GP {
    // Params for evolution
    int   populationSize = 200;  // Number of agents per round
    int   selectionSize  = 10;    // Number of agents in the tournament selection
    int   maxDepth       = 10;    // Max Depth of classification tree
    int   maxGenerations = 100000; // Max number of generations
    int   numBreed       = 10;    // Number of new agents during breeding
    int   numAdopt       = 20;    // Number of new agents to adopt
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
        for(int i = 0; i < num; i++) {
            agents.push_back(std::make_unique<Agent>(Agent()));
            agents.at(agents.size() - 1)->setRandomDNAStrain([&](bool isRoot, int currDepth) -> gene* {
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

    void fitnessAgents(vector<dataset>& data) {
        vector<string> answers;
        for(auto &d : data) {
            answers.push_back(d.label);
        }
        auto fitnessTest = [&](int index, int ans) {
            //PrettyPrint(index);
            //PrettyPrint(answers.size());
            if(ans >= 0 && answers.at(index) == "ped") {
                return true;
            }
            if(ans < 0 && answers.at(index) == "non-ped") {
                return true;
            };
            return false;
        };
#if !ASYNC_CODE
        for(auto &a : agents) {
            a->calcSimpleFitness(fitnessTest);
        }
#else
        std::vector< std::future<void> > voidFutures;
        for(auto &a : agents) {
            voidFutures.push_back(std::async(std::launch::async,
                [&](){
                    a->calcSimpleFitness(fitnessTest);
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

    gene* mutFunc(gene* g) {
        return g;
    }

    Agent crossover(std::unique_ptr<Agent>::pointer a1, std::unique_ptr<Agent>::pointer a2) {
        Agent newAgent = Agent();
        DNA * newDNA = new DNA();
        if(rand()%10 > 5) {
            copyGeneTree(a1->dna->gRoot, newDNA->gRoot, mutFunc);
            copyGeneTree(a2->dna->gRoot->l, newDNA->gRoot->l, mutFunc);
        } else {
            copyGeneTree(a2->dna->gRoot, newDNA->gRoot, mutFunc);
            copyGeneTree(a1->dna->gRoot->l, newDNA->gRoot->l, mutFunc);
        }
        newAgent.dna.reset(newDNA);
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

        GP::fitnessAgents(data);
        std::sort(GP::agents.begin(), GP::agents.end(), [](std::unique_ptr<Agent>& one, std::unique_ptr<Agent>& two){
            return one->fitness > two->fitness;
        });
        for(int j = 0; j < 10; j++) {
            PrettyPrint(GP::agents.at(j)->getFitness());
        }
        cout << GP::agents.at(0)->dna << endl;

        GP::Breed();
        GP::resetAgents();
    }
}
