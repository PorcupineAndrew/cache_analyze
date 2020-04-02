#include "macro.h"
#include "cache.h"
#include "mem.h"

void error_handler(int sig) {
    void *arr[10];
    size_t size = backtrace(arr, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(arr, size, STDERR_FILENO);
    exit(1);
}

/* arguments:
 *  - input file path
 *  - output file path
 *  - cache size in bit width
 *  - block size in bit width
 *  - way size in bit width
 *  - replace algorithm 
 *  - write assign or not
 *  - write back or not
 **/
int main(int argc, char** argv) {
    signal(SIGABRT, error_handler);

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

    Cache myCache (17, 3, 14, GET_STRATEGY(REPLACE_LRU, WRITE_ASSIGN, WRITE_BACK));

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
