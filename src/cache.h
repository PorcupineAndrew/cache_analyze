#ifndef _CACHE_H_
#define _CACHE_H_
#include "macro.h"

class Cache {
private:
    const int numBitTotoalSize;
    const int numBitBlockSize;
    const int numBitWay;
    const int numBitGroup;
    const int numCacheLine;

    int numHit;
    int numAccess;

    cache_line_t *cacheLines;

    void updateCacheLine(int idx, addr_t addr);
    bool cacheLineValid(int idx);
    void setCacheLineValid(int idx);
    bool cacheLineDirty(int idx);
    void setCacheLineDirty(int idx);
    addr_t getCacheLineTag(int idx);

public:
    Cache(int ts, int bs, int nw);
    ~Cache();

    void write(addr_t addr);
    void read(addr_t addr);
};

#endif
