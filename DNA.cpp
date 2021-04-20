#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <memory>
#include <functional>

enum op { Plus = 0, Minus = 1, Mul = 2, Div = 3, If = 4, Constant = 5, Feature = 6 };

struct gene {
    gene(bool isRoot, op o, float val) {
        root  = isRoot;
        type  = o;
        value = val;
    }

    gene(bool isRoot, op o, float val, gene* le, gene* mi, gene* ri) {
        root  = isRoot;
        type  = o;
        value = val;
        l.reset(le);
        m.reset(mi);
        r.reset(ri);
    }
    bool root = false;
    op type;
    float value;
    std::unique_ptr<gene> l = NULL;
    std::unique_ptr<gene> m = NULL; // Will not be null if operator is if
    std::unique_ptr<gene> r = NULL;
};

class DNA {
    public:
        std::unique_ptr<gene> gRoot;

        DNA() {}

        //DNA(std::unique_ptr<gene> pRoot) {
            //gRoot = pRoot;
        //}

        ~DNA() {
            // @TODO Clean up all other genes starting from root.
        }
};


class Agent {
    private:

    public:
        float fitness;
        std::vector<float> classifications;
        std::unique_ptr<DNA> dna = nullptr;

        Agent() {}

        /**
         * Builds a random DNA strain
         */
        //template <typename T>
        void setRandomDNAStrain(std::function<gene*(bool, int)> randomGeneGen) {
            std::function<void(std::unique_ptr<gene>&, int)> rec = [&](std::unique_ptr<gene>& currGene, int d) -> void {
                switch(currGene->type) {
                    case Plus:
                    case Minus:
                    case Mul:
                    case Div:
                        currGene->l.reset(randomGeneGen(false, d+1));
                        rec(currGene->l, d+1);
                        currGene->r.reset(randomGeneGen(false, d+1));
                        rec(currGene->r, d+1);
                        break;
                    case If:
                        currGene->l.reset(randomGeneGen(false, d+1));
                        rec(currGene->l, d+1);
                        currGene->r.reset(randomGeneGen(false, d+1));
                        rec(currGene->r, d+1);
                        currGene->m.reset(randomGeneGen(false, d+1));
                        rec(currGene->m, d+1);
                        break;
                    case Constant:
                        return;
                    case Feature:
                        return;
                }
            };
            DNA * ret = new DNA();
            ret->gRoot.reset(randomGeneGen(true, 0));
            rec(ret->gRoot, 0);
            dna.reset(ret);
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
        //template <typename T>
        float classification(std::function<float(int)> featureTranslator) {
            std::function<float(std::unique_ptr<gene>&)> rec = [&](std::unique_ptr<gene>& currGene) -> float {
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
            return score;
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

        template<typename T>
        float calcAverageFitness(T fitnessTest) {
            if(classifications.size() == 0) {
                std::cout << "DNA: Classifications is 0" << std::endl;
                return -1;
            }
            int TP = 0, TN = 0, FN = 0, FP = 0;
            int i = 0;
            for(const auto & c : classifications) {
                switch(fitnessTest(i, c)) {
                    case 0:
                        TP += 1;
                        break;
                    case 1:
                        TN += 1;
                        break;
                    case 2:
                        FN += 1;
                        break;
                    case 3:
                        FP += 1;
                        break;
                }
                i += 1;
            }
            //fitness = float(numCorrect) / float(classifications.size());
            fitness = ((float(TP)/(TP+FN)) + (float(TN)/(TN+FP))) / 2;
            return fitness;
        }

        float getFitness() {
            return fitness;
        }
};

gene* copyGene( std::unique_ptr<gene> from ) {
    if(from == NULL) {
        return NULL;
    }
    return (new gene(from->root, from->type, from->value, from->l.get(), from->m.get(), from->r.get()));
}

template<typename T>
void copyGeneTree( std::unique_ptr<gene> from, std::unique_ptr<gene>& to, T mutFunc ) {
    if(from != NULL) {
        to = std::make_unique<gene>(copyGene(from));
        //to->l = std::make_unique<gene>(mutFunc(copyGene(from->l)));
        copyGeneTree(from->l, to->l, mutFunc);
        //to->m = std::make_unique<gene>(mutFunc(copyGene(from->m)));
        copyGeneTree(from->m, to->m, mutFunc);
        //to->r = std::make_unique<gene>(mutFunc(copyGene(from->r)));
        copyGeneTree(from->r, to->r, mutFunc);
    }
}

std::ostream& operator<<(std::ostream& os, const std::unique_ptr<gene> node) {
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
    std::vector<std::unique_ptr<gene>::pointer> nodeList;
    nodeList.push_back(dt->gRoot.get());
    while(nodeList.size() != 0) {
        std::vector< std::unique_ptr<gene>::pointer > newNodeList;
        for(const auto & node : nodeList) {
            os << node << " ";
            if(node->l != nullptr) {
                newNodeList.push_back(node->l.get());
            }
            if(node->m != nullptr) {
                newNodeList.push_back(node->m.get());
            }
            if(node->r != nullptr) {
                newNodeList.push_back(node->r.get());
            }
        }
        nodeList = newNodeList;
        os << std::endl;
        depth += 1;
    }
    return os;
}


//int main() {
//}
