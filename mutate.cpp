#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <memory>
#include <functional>


using std::cout;
using std::endl;
using std::vector;
using std::string;

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


enum op { Plus = 0, Minus = 1, Mul = 2, Div = 3, If = 4, Constant = 5, Feature = 6 };

#define print(x) std::cout << #x << " : " << x << std::endl;



struct gene {
    gene(bool isRoot, op o, float val) {
        root  = isRoot;
        type  = o;
        value = val;
    }

    bool root = false;
    op type;
    float value;
    std::unique_ptr<gene> l = NULL;
    std::unique_ptr<gene> m = NULL; // Will not be null if operator is if
    std::unique_ptr<gene> r = NULL;
};

gene* mutFunc(gene* g, int gDepth){
    float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    std::function<gene*(int)> makeRandomGene = [&](int depth){
        float opRand = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        int value = 0;
        int type = 0;
        print(opRand);
        if(depth == maxDepth){
            while(opRand < .4){
                opRand = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            }
        }
        print(opRand);
        if(opRand < .32){ //+-*/
            g->l.reset(makeRandomGene(depth+1));
            g->r.reset(makeRandomGene(depth+1));
            if(opRand < .8){
                type = 0;
            } else if(opRand > .8 && opRand < .16){
                type = 1;
            } else if(opRand > .16 && opRand < .24){
                type = 2;
            } else if(opRand > .24){
                type = 3;
            }
        } else if(opRand > .32 && opRand < .4){ //if
            g->l.reset(makeRandomGene(depth+1));
            g->m.reset(makeRandomGene(depth+1));
            g->r.reset(makeRandomGene(depth+1));
            type = 4;
        } else if(opRand > .4 && opRand < .5){ //constant
            value = rand() % maxConst + 1;
            type = 5;
        } else if(opRand > .5){ //feature
            value = rand() % 23;
            type = 6;
        }
        return new gene(g->root,static_cast <op>(type),value);
    };
  //  if(randNum < mutationRate){
        g = makeRandomGene(gDepth);
  //  }
    return g;
}


int main(){
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    gene * fakeGene = new gene(false, op::Minus,2);
    rand();
    fakeGene = mutFunc(fakeGene, 0);
}