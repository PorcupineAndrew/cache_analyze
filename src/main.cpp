#include "macro.h"
#include "cache.h"

Cache myCache (17, 3, 2, GET_STRATEGY(REPLACE_LRU, WRITE_ASSIGN, WRITE_BACK));

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "argument invalid" << endl;
        exit(0);
    }

    string fName = argv[1];
    ifstream inFile (fName);

    if (inFile.is_open()) {
        int numLine = 0;
        addr_t addr;
        string op, line;
        while (getline(inFile, line)) {
            stringstream(line) >> op >> hex >> addr;
            // if (op == "r") cout << "read:\t0x" << STREAM_ADDR(addr) << endl;
            // else cout << "write:\t0x" << STREAM_ADDR(addr) << endl;
            // if (++numLine == 10) break;
            if (op == "r") myCache.read(addr);
            else myCache.write(addr);
            if (++numLine == 1000) break;
        }
        myCache.print();
    } else 
        cout << "failed to open file " << fName << endl;

    return 0;
}
