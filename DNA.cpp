#include <stdlib.h>
#include <iostream>
#include <vector>

enum op { Plus = 0, Minus = 1, Mul = 2, Div = 3, If = 4, Constant = 5, Feature = 6 };

struct nucleotide {
    bool root = false;
    op type;
    float value;
    nucleotide * l = nullptr;
    nucleotide * m = nullptr; // Will be null if operator is if
    nucleotide * r = nullptr;
};

class DNA {
    public:
        nucleotide * gRoot;

        DNA() {
            gRoot = new nucleotide();
        }

        DNA(nucleotide * pRoot) {
            gRoot = pRoot;
        }
        ~DNA() {}

        template<typename T>
        float classify(T FeatureTranslator) {
        }

        template<typename T>
        float fitnessTest(T correctnessFunc) {
            if(classifications.size() == 0) {
                std::cout << "DNA: Classifications is 0" << std::endl;
                return -1;
            }
            int numCorrect = 0;
            for(const auto & c : classifications) {
                if(correctnessFunc(c)) {
                    numCorrect += 1;
                }
            }
            fitness = float(numCorrect) / float(classifications.size());
            return fitness;
        }

        float getFitness() {
            return fitness;
        }

    private:
        std::vector<float> classifications;
        float fitness;
};

void buildRandomDNA() {

}

int main() {
    DNA dna;
    auto testClassify = [](int c){
        switch(c) {
            case 0:
                return 0;
            case 1:
                return 10;
            case 2:
                return 20;
            case 3:
                return 30;
            default:
                return -5;
        }
    };
    dna.classify(testClassify);
    dna.fitnessTest([](float x){return false;});
}
