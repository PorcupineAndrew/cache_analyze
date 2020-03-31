#include "cache.h"
#include <bitset>

Cache::Cache(int ts, int bs, int nw, cache_strategy_t st) : 
    numBitTotoalSize(ts), numBitBlockSize(bs), numBitWay(nw), numBitGroup(ts-bs-nw),
    numCacheLine(POWER2(ts-bs)), strategy(st), numHit(0), numAccess(0) {

    cout << "Cache init\n"
         << "--------------------------------\n"
         << "total size  : " << POWER2(this->numBitTotoalSize) << "\n"
         << "block size  : " << POWER2(this->numBitBlockSize) << "\n"
         << "way number  : " << POWER2(this->numBitWay) << "\n"
         << "group number: " << POWER2(this->numBitGroup) << "\n"
         << "cache lines : " << this->numCacheLine << "\n"
         << "write back  : " << isWriteBack() << "\n"
         << "write direct: " << isWriteDirect() << "\n"
         << "write assign: " << isWriteAssign() << "\n"
         << "replace algo: " << (isLRU() ? "LRU" : (isRAND() ? "RAND" : "TREE"))<< "\n"
         << "--------------------------------" << endl;

    this->cacheLines = new cache_line_t[this->numCacheLine];
    memset(this->cacheLines, 0, this->numCacheLine*sizeof(cache_line_t));

    if (isLRU()) {
        this->lruRecords = new lru_record_t[POWER2(this->numBitGroup)];
        memset(this->lruRecords, 0, POWER2(this->numBitGroup)*sizeof(lru_record_t));
    }
    if (isTREE()) {
        this->treeRecords = new tree_record_t[POWER2(this->numBitGroup)];
        memset(this->treeRecords, 0, POWER2(this->numBitGroup)*sizeof(tree_record_t));
    }

    cout << "addr length : " << ADDR_BIT << "\n"
         << "valid pos   : " << VALID_POS << "\n"
         << "dirt pos    : " << DIRT_POS << "\n"
         << "--------------------------------" << endl;
}

Cache::~Cache() {
    delete [] this->cacheLines;
    if (this->lruRecords) delete [] this->lruRecords;
    if (this->treeRecords) delete [] this->treeRecords;
}

void Cache::updateCacheLine(int idx, addr_t addr) {
    const int& offsetLen = this->numBitBlockSize;
    const int& indexLen = this->numBitGroup;

    addr_t buf = SLICE(addr, offsetLen+indexLen, ADDR_BIT);
    SETBIT(buf, VALID_POS);
    memcpy(&this->cacheLines[idx], &buf, sizeof(cache_line_t));
}

bool Cache::cacheLineValid(int idx) const {
    return *(((char*) &this->cacheLines[idx]) + sizeof(cache_line_t) - 1) < 0;
}

bool Cache::cacheLineDirty(int idx) const {
    return (*(((char*) &this->cacheLines[idx]) + sizeof(cache_line_t) - 1) << 1) < 0;
}

addr_t Cache::getCacheLineTag(int idx) {
    const int& offsetLen = this->numBitBlockSize;
    const int& indexLen = this->numBitGroup;

    addr_t buf = 0;
    memcpy(&buf, &this->cacheLines[idx], sizeof(cache_line_t));
    return SLICE(buf, 0, ADDR_BIT-offsetLen-indexLen);
}

void Cache::read(addr_t addr) {
    this->numAccess += 1;
    const int& offsetLen = this->numBitBlockSize;
    const int& indexLen = this->numBitGroup;
    const int numWay = POWER2(this->numBitWay);

    unsigned int index = SLICE(addr, offsetLen, offsetLen+indexLen);
    index <<= this->numBitWay;
    unsigned long long int tag = SLICE(addr, offsetLen+indexLen, ADDR_BIT);

    int state = -1;
    for (int i = 0; i < numWay; i++) {
        if (cacheLineValid(index+i)) {
            if (getCacheLineTag(index+i) == tag) {
                hit(index+i);
                state = -2;
                break;
            }
        } else state = i;
    }
    if (state != -2) {
        if (state >= 0) {
            updateLRU(index+state);
            updateCacheLine(index+state, addr);
        } else {
            int idx = purge(index);
            updateCacheLine(idx, addr);
        }
    }
}

void Cache::write(addr_t addr) {
    this->numAccess += 1;
    const int& offsetLen = this->numBitBlockSize;
    const int& indexLen = this->numBitGroup;
    const int numWay = POWER2(this->numBitWay);

    unsigned int index = SLICE(addr, offsetLen, offsetLen+indexLen);
    index <<= this->numBitWay;
    unsigned long long int tag = SLICE(addr, offsetLen+indexLen, ADDR_BIT);
    
    int state = -1;
    for (int i = 0; i < numWay; i++) {
        if (cacheLineValid(index+i)) {
            if (getCacheLineTag(index+i) == tag) {
                hit(index+i);
                // setCacheLineDirty(index+i);
                state = -2;
                break;
            }
        } else state = i;
    }
    if (state != -2) {
        if (state >= 0) {
            // updateCacheLine(state, addr);
        } else {
            // TODO
        }
    }
}

void Cache::setCacheLineValid(int idx) {
    SETBIT(*((addr_t *)&this->cacheLines[idx]), VALID_POS);
}

void Cache::unsetCacheLineValid(int idx) {
    UNSETBIT(*((addr_t *)&this->cacheLines[idx]), VALID_POS);
}

void Cache::setCacheLineDirty(int idx) {
    SETBIT(*((addr_t *)&this->cacheLines[idx]), DIRT_POS);
}

bool Cache::isWriteAssign() const {
    return ((this->strategy >> WRITE_ASSIGN_POS) & 1U) == WRITE_ASSIGN;
}

bool Cache::isWriteNoAssign() const {
    return ((this->strategy >> WRITE_ASSIGN_POS) & 1U) == WRITE_NO_ASSIGN;
}

bool Cache::isWriteBack() const {
    return ((this->strategy >> WRITE_BACK_POS) & 1U) == WRITE_BACK;
}

bool Cache::isWriteDirect() const {
    return ((this->strategy >> WRITE_BACK_POS) & 1U) == WRITE_DIRECT;
}

bool Cache::isLRU() const {
    return ((this->strategy >> REPLACE_POS) & 3U) == REPLACE_LRU;
}

bool Cache::isRAND() const {
    return ((this->strategy >> REPLACE_POS) & 3U) == REPLACE_RAND;
}

bool Cache::isTREE() const {
    return ((this->strategy >> REPLACE_POS) & 3U) == REPLACE_TREE;
}

void Cache::hit(int idx) {
    this->numHit += 1;
    if (isLRU()) {
        updateLRU(idx);
    } else if (isTREE()) {
        // TODO
    }
}

int Cache::purge(int index) {
    if (isLRU()) {
        const int numWay = POWER2(this->numBitWay);
        unsigned int buf = 0;
        memcpy(&buf, &this->lruRecords[index >> this->numBitWay], sizeof(lru_record_t));

        int idx = -1, max = 0;
        for (int i = 0; i < numWay; i++) {
            int rank = getLruRank(buf, i);
            if (rank > max) {
                max = rank;
                idx = index+i;
            }
        }

        assert (max == numWay);
        updateLRU(idx);
        return idx;
    } else if (isRAND()) {
        return index + rand() % POWER2(this->numBitWay);
    } else if (isTREE()) {
        return 0;
    } else {
        cout << "Error on repalce" << endl;
        exit(0);
    }
}

void Cache::updateLRU(int idx) {
    const int index = (idx >> this->numBitWay) << this->numBitWay, numWay = POWER2(this->numBitWay);
    unsigned int buf = 0, mask = MASK(this->numBitWay);
    memcpy(&buf, &this->lruRecords[index >> this->numBitWay], sizeof(lru_record_t));

    unsigned int upperRank = cacheLineValid(idx-index) ? 
                            getLruRank(buf, idx-index) : (1U<<this->numBitWay);

    for (int i = 0; i < numWay; i++, mask <<= this->numBitWay) {
        if (index+i == idx) {
            buf &= (~mask);
            continue;
        }
        if (cacheLineValid(index+i) && (getLruRank(buf, i) < upperRank)) {
            buf += (1U << (this->numBitWay * i));
        }
    }
    memcpy(&this->lruRecords[index >> this->numBitWay], &buf, sizeof(lru_record_t));
}

unsigned int Cache::getLruRank(unsigned int buf, int i) const {
    int offset =  i*this->numBitWay;
    unsigned int mask = MASK(this->numBitWay);
    return (buf & (mask << offset)) >> offset;
}

void Cache::print() {
    cout << "total access: " << this->numAccess << "\n"
         << "hit times   : " << this->numHit << "\n"
         << "--------------------------------" << endl;
}
