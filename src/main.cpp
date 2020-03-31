#include "macro.h"
#include "cache.h"
#include "mem.h"

Cache myCache (17, 3, 2, GET_STRATEGY(REPLACE_TREE, WRITE_ASSIGN, WRITE_BACK));

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "argument invalid" << endl;
        exit(1);
    }

    string fName = argv[1], oName = argv[2];
    ifstream inFile (fName);
    if (freopen(oName.c_str(), "w", stdout) == NULL) {
        cerr << "fail to redirect stdout" << endl;
        exit(1);
    }

    if (inFile.is_open()) {
        addr_t addr;
        string op, line;
        int isHit;
        while (getline(inFile, line)) {
            stringstream(line) >> op >> hex >> addr;
            if (op == "r")
                isHit = myCache.read(addr);
            else
                isHit = myCache.write(addr);
            cout << (isHit ? "Hit" : "Miss") << endl;
        }
        myCache.print();
    } else 
        cerr << "failed to open file " << fName << endl;

    return 0;
}
