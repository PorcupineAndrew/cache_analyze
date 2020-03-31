#include "macro.h"
#include "cache.h"

Cache myCache (17, 3, 2, GET_STRATEGY(REPLACE_TREE, WRITE_ASSIGN, WRITE_BACK));

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "argument invalid" << endl;
        exit(0);
    }

    string fName = argv[1];
    ifstream inFile (fName);

    if (inFile.is_open()) {
        addr_t addr;
        string op, line;
        while (getline(inFile, line)) {
            stringstream(line) >> op >> hex >> addr;
            if (op == "r") myCache.read(addr);
            else myCache.write(addr);
        }
        myCache.print();
    } else 
        cout << "failed to open file " << fName << endl;

    return 0;
}
