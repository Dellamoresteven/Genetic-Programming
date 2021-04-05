#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <vector>

enum op { Plus = 0, Minus = 1, Mul = 2, Div = 3, If = 4, Constant = 5, Feature = 6 };

struct gene {
    bool root = false;
    op type;
    float value;
    gene * l = nullptr;
    gene * m = nullptr; // Will be null if operator is if
    gene * r = nullptr;
};

class DNA {
    public:
        gene * gRoot;

        DNA() {
            gRoot = new gene();
            gRoot->root = true;
        }

        DNA(gene * pRoot) {
            gRoot = pRoot;
        }

        ~DNA() {
            // @TODO Clean all other genes starting from root.
        }
};


class Agent {
    private:
        DNA * dna;
        float fitness;
        std::vector<float> classifications;


    public:
        Agent() {}
        Agent(DNA * dna) {
            dna = dna;
            srand((unsigned) time(NULL));
        }

        /**
         * Builds a random DNA strain
         */
        template <typename T>
        void setRandomDNAStrain(T randomizationValues) {
            auto rec = [&](gene * currGene) -> auto {
                float randNum = static_cast<float> (rand()) / (static_cast<float>(RAND_MAX));
                gene newGene = randomizationValues(randNum);
                switch(newGene.type) {
                    case Plus:
                        break;
                    case Minus:
                        break;
                    case Mul:
                        break;
                    case Div:
                        break;
                    case If:
                        break;
                    default:
                        std::cout << "Something went wrong" << std::endl;
                        exit(1);
                }
                rec(currGene->l);
                rec(currGene->m);
                rec(currGene->r);
                return 0;
            };
            DNA * ret = new DNA();
            rec(ret->gRoot);
            dna = ret;
        }

        /**
         * Resets all vars in class
         */
        void reset() {
            classifications.clear();
            fitness = 0;
        }

        /**
         * Classify's each object
         *
         * @param featureTranslator is a function that takes in a feature int and returns
         *        the real feature value.
         */
        template <typename T>
        void classification(T featureTranslator) {
        }

        /**
         * Calculates the fitness based on the current classifications. The score is
         * equal to right answers over wrong answers.
         *
         * @param: A function that takes in a float and returns a boolean telling us
         *         if we got it wrong (false) or right (true).
         *
         * @return a fitness score (float)
         */
        template<typename T>
        float calcSimpleFitness(T fitnessTest) {
            if(classifications.size() == 0) {
                std::cout << "DNA: Classifications is 0" << std::endl;
                return -1;
            }
            int numCorrect = 0;
            for(const auto & c : classifications) {
                if(fitnessTest(c)) {
                    numCorrect += 1;
                }
            }
            fitness = float(numCorrect) / float(classifications.size());
            return fitness;
        }

        float getFitness() {
            return fitness;
        }
};

/**
 * DNA print override
 */
std::ostream& operator<<(std::ostream& os, const DNA& dt) {
}


int main() {
    Agent * a = new Agent();
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
    a->calcSimpleFitness([](float x){return false;});
}
