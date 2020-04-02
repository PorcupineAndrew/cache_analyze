#ifndef _CACHE_H_
#define _CACHE_H_
#include "macro.h"
#include "mem.h"

class Cache {
private:
    const int numBitTotoalSize;
    const int numBitBlockSize;
    const int numBitWay;
    const int numBitGroup;
    const int numCacheLine;

    const int& numBitOffset;
    const int& numBitIndex;
    const int numBitTag;

    const int validBit;
    const int dirtyBit;

    const cache_strategy_t strategy;

    int numHit;
    int numAccess;
    int numPurge;


    Mem *cacheLines = nullptr;
    Mem *lruRecords = nullptr;
    Mem *treeRecords = nullptr;

    void updateCacheLine(int idx, addr_t addr);

    bool cacheLineValid(int idx) const;
    void setCacheLineValid(int idx);
    void unsetCacheLineValid(int idx);

    bool cacheLineDirty(int idx) const;
    void setCacheLineDirty(int idx);

    addr_t getCacheLineTag(int idx);

    bool isWriteAssign() const;
    bool isWriteNoAssign() const;
    bool isWriteBack() const;
    bool isWriteDirect() const;
    bool isLRU() const;
    bool isRAND() const;
    bool isTREE() const;

    int purge(int index);
    void updateLRU(int idx);
    void updateTREE(int idx);
    void hit(int idx);
public:
    Cache(int ts, int bs, int nw, cache_strategy_t st);
    ~Cache();

    int write(addr_t addr);
    int read(addr_t addr);
    void print();
};

#endif
