#include <stdlib.h>
#include <iostream>

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

    private:
};

int main() {
    DNA dna;
}
