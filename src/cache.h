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
    const cache_strategy_t strategy;

    int numHit;
    int numAccess;
    int numPurge;

    cache_line_t *cacheLines = nullptr;
    lru_record_t *lruRecords = nullptr;
    tree_record_t *treeRecords = nullptr;

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
    unsigned int getLruRank(unsigned int buf, int idx) const;
    void updateLRU(int idx);
    void updateTREE(int idx);
    void hit(int idx);
public:
    Cache(int ts, int bs, int nw, cache_strategy_t st);
    ~Cache();

    void write(addr_t addr);
    void read(addr_t addr);
    void print();
};

#endif
