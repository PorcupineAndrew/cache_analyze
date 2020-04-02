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

string getCmdOption(char** begin, char** end, const string& option) {
    char** iter = find(begin, end, option);
    if (iter != end && ++iter != end) {
        return string(*iter);
    }
    return "";
}

bool cmdOptionExists(char** begin, char** end, const string& option) {
    return find(begin, end, option) != end;
}


/* Arguments:
 *  --input_file    : input file path,          [string]
 *  --output_file   : output file path,         [string]
 *  --cache_size_bit: cache size in bit width,  [integer]
 *  --block_size_bit: block size in bit width,  [integer]
 *  --way_size_bit  : way size in bit width,    [integer or string]
 *  --replace_algo  : replace algorithm ,       [string]
 *  --write_assgin  : write assign,             [flag]
 *  --write_back    : write back,               [flag]
 **/
int main(int argc, char** argv) {
    signal(SIGABRT, error_handler);

    string fName        = getCmdOption(argv, argv+argc, "--input_file"),
           oName        = getCmdOption(argv, argv+argc, "--output_file");
    int    cacheSizeBit = stoi(getCmdOption(argv, argv+argc, "--cache_size_bit")),
           blockSizeBit = stoi(getCmdOption(argv, argv+argc, "--block_size_bit")),
           waySizeBit   = getCmdOption(argv, argv+argc, "--way_size_bit") == "DM" ?
                        1 : (getCmdOption(argv, argv+argc, "--way_size_bit") == "FA" ?
                        cacheSizeBit-blockSizeBit : 
                        stoi(getCmdOption(argv, argv+argc, "--way_size_bit"))),
           replaceAlgo  = getCmdOption(argv, argv+argc, "--replace_algo") == "lru" ?
                        REPLACE_LRU : (getCmdOption(argv, argv+argc, "--replace_algo") == "rand" ?
                        REPLACE_RAND : REPLACE_TREE),
           writeAssign  = cmdOptionExists(argv, argv+argc, "--write_assgin") ?
                        WRITE_ASSIGN : WRITE_NO_ASSIGN,
           writeBack    = cmdOptionExists(argv, argv+argc, "--write_back") ?
                        WRITE_BACK : WRITE_DIRECT;

    ifstream inFile (fName);
    if (freopen(oName.c_str(), "w", stdout) == NULL) {
        cerr << "fail to redirect stdout" << endl;
        exit(1);
    }

    Cache myCache (cacheSizeBit, blockSizeBit, waySizeBit, 
                    GET_STRATEGY(replaceAlgo, writeAssign, writeBack));

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
