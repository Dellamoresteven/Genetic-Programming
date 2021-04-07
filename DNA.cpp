#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <functional>

enum op { Plus = 0, Minus = 1, Mul = 2, Div = 3, If = 4, Constant = 5, Feature = 6 };

struct gene {
    gene(bool isRoot, op o, float val) {
        root  = isRoot;
        type  = o;
        value = val;
    }
    bool root = false;
    op type;
    float value;
    gene * l = NULL;
    gene * m = NULL; // Will be null if operator is if
    gene * r = NULL;
};

class DNA {
    public:
        gene * gRoot;

        DNA() {}

        DNA(gene * pRoot) {
            gRoot = pRoot;
        }

        ~DNA() {
            // @TODO Clean up all other genes starting from root.
        }
};


class Agent {
    private:

    public:
        float fitness;
        std::vector<float> classifications;
        DNA * dna;

        Agent() {}
        Agent(DNA * d) {
            dna = d;
        }

        /**
         * Builds a random DNA strain
         */
        template <typename T>
        void setRandomDNAStrain(T randomGeneGen) {
            std::function<void(gene*)> rec = [&](gene * currGene) -> void {
                switch(currGene->type) {
                    case Plus:
                    case Minus:
                    case Mul:
                    case Div:
                        currGene->l = randomGeneGen(false);
                        rec(currGene->l);
                        currGene->r = randomGeneGen(false);
                        rec(currGene->r);
                        break;
                    case If:
                        currGene->l = randomGeneGen(false);
                        rec(currGene->l);
                        currGene->r = randomGeneGen(false);
                        rec(currGene->r);
                        currGene->m = randomGeneGen(false);
                        rec(currGene->m);
                        break;
                    case Constant:
                        return;
                    case Feature:
                        return;
                }
            };
            DNA * ret = new DNA();
            ret->gRoot = randomGeneGen(true);
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
            std::function<float(gene*)> rec = [&](gene * currGene) -> float {
                switch(currGene->type) {
                    case Plus:
                        return rec(currGene->l) + rec(currGene->r);
                    case Minus:
                        return rec(currGene->l) - rec(currGene->r);
                    case Mul:
                        return rec(currGene->l) * rec(currGene->r);
                    case Div:
                        {
                            float r = rec(currGene->r);
                            if(r == 0) {
                                return 0.0;
                            }
                            return rec(currGene->l) / float(r);
                        }
                    case If:
                        return (rec(currGene->l) >= 0) ? rec(currGene->m) : rec(currGene->r);
                    case Feature:
                        return featureTranslator(currGene->value);
                    case Constant:
                        return currGene->value;
                }
                return 0.0;
            };
            float score = rec(dna->gRoot);
            classifications.push_back(score);
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
            int i = 0;
            for(const auto & c : classifications) {
                if(fitnessTest(i, c)) {
                    numCorrect += 1;
                }
                i += 1;
            }
            fitness = float(numCorrect) / float(classifications.size());
            return fitness;
        }

        float getFitness() {
            return fitness;
        }
};

gene * copyGene( gene * from ) {
    if(from == NULL) {
        return NULL;
    }
    return (new gene(from->root, from->type, from->value));
}

void copyGeneTree( gene * from, gene *& to, float mutRate ) {
    if(from != NULL) {
        to = copyGene(from);
        gene * newLeft = copyGene(from->l);
        to->l = newLeft;
        copyGeneTree(from->l, to->l, mutRate);
        gene * newMiddle = copyGene(from->m);
        to->m = newMiddle;
        copyGeneTree(from->m, to->m, mutRate);
        gene * newRight = copyGene(from->r);
        to->r = newRight;
        copyGeneTree(from->r, to->r, mutRate);
    }
}

std::ostream& operator<<(std::ostream& os, const gene* node) {
    switch(static_cast<int>(node->type)) {
        case Plus:
            os << "+";
            break;
        case Minus:
            os << "-";
            break;
        case Mul:
            os << "*";
            break;
        case Div:
            os << "%";
            break;
        case If:
            os << "If";
            break;
        case Constant:
            os << node->value;
            break;
        case Feature:
            os << "F" << node->value;
            break;
    }
    return os;
}

/**
 * DNA print override
 */
std::ostream& operator<<(std::ostream& os, const DNA* dt) {
    int depth = 0;
    std::vector<gene*> nodeList;
    nodeList.push_back(dt->gRoot);
    while(nodeList.size() != 0) {
        std::vector<gene*> newNodeList;
        for(const auto & node : nodeList) {
            os << node << " ";
            if(node->l != nullptr) {
                newNodeList.push_back(node->l);
            }
            if(node->m != nullptr) {
                newNodeList.push_back(node->m);
            }
            if(node->r != nullptr) {
                newNodeList.push_back(node->r);
            }
        }
        nodeList = newNodeList;
        os << std::endl;
        depth += 1;
    }
    return os;
}


//int main() {
    //srand (time(NULL));
    //Agent * a = new Agent();
    //auto testClassify = [](int c){
        //switch(c) {
            //case 0:
                //return 0;
            //case 1:
                //return 10;
            //case 2:
                //return 20;
            //case 3:
                //return 30;
            //default:
                //return -5;
        //}
    //};
    ////a->calcSimpleFitness([](float x){return false;});
    //a->setRandomDNAStrain([&]() -> gene* {
        //float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        //if(randNum < .3) {
            //return new gene(false, static_cast<op>(rand()%5), 0);
        //} else if(randNum < (.3 + .15)) {
            //return new gene(false, op::Constant, rand()%20);
        //} else {
            //return new gene(false, op::Feature, rand()%10);
        //}
    //});
    //std::cout << a->dna << std::endl;
    //a->classification([](int featureNumber){
        //return float(featureNumber);
    //});
//}
