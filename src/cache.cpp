#include "cache.h"
#include <bitset>

Cache::Cache(int ts, int bs, int nw) : 
    numBitTotoalSize(ts), numBitBlockSize(bs), numBitWay(nw), numBitGroup(ts-bs-nw),
    numCacheLine(POWER2(ts-bs)), numHit(0), numAccess(0) {

    cout << "Cache init\n"
         << "--------------------------------\n"
         << "total size  : " << POWER2(this->numBitTotoalSize) << "\n"
         << "block size  : " << POWER2(this->numBitBlockSize) << "\n"
         << "way number  : " << POWER2(this->numBitWay) << "\n"
         << "group number: " << POWER2(this->numBitGroup) << "\n"
         << "cache lines : " << this->numCacheLine << "\n"
         << "--------------------------------" << endl;

    this->cacheLines = new cache_line_t[this->numCacheLine];
    memset(this->cacheLines, 0, this->numCacheLine*sizeof(cache_line_t));

    cout << "addr length : " << ADDR_BIT << "\n"
         << "valid pos   : " << VALID_POS << "\n"
         << "dirt pos    : " << DIRT_POS << "\n"
         << "--------------------------------" << endl;
}

Cache::~Cache() {
    delete this->cacheLines;
}

void Cache::updateCacheLine(int idx, addr_t addr) {
    const int& offsetLen = this->numBitBlockSize;
    const int& indexLen = this->numBitGroup;

    unsigned long long int buf = SLICE(addr, offsetLen+indexLen, ADDR_BIT);
    SETBIT(buf, VALID_POS);
    memcpy(&this->cacheLines[idx], &buf, sizeof(cache_line_t));
}

bool Cache::cacheLineValid(int idx) {
    return *(((char*) &this->cacheLines[idx]) + sizeof(cache_line_t) - 1) < 0;
}

bool Cache::cacheLineDirty(int idx) {
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
    unsigned long long int tag = SLICE(addr, offsetLen+indexLen, ADDR_BIT);
    unsigned int idx = index<<this->numBitWay;

    int state = -1;
    for (int i = 0; i < numWay; i++) {
        if (cacheLineValid(idx+i)) {
            if (getCacheLineTag(idx+i) == tag) {
                this->numHit += 1;
                state = -2;
                break;
            }
        } else state = i;
    }
    if (state != -2) {
        if (state >= 0) {
            updateCacheLine(idx+state, addr);
        } else {
            // TODO
        }
    }
    
}

void Cache::write(addr_t addr) {
    this->numAccess += 1;
    const int& offsetLen = this->numBitBlockSize;
    const int& indexLen = this->numBitGroup;
    const int numWay = POWER2(this->numBitWay);

    unsigned int index = SLICE(addr, offsetLen, offsetLen+indexLen);
    unsigned long long int tag = SLICE(addr, offsetLen+indexLen, ADDR_BIT);
    unsigned int idx = index<<this->numBitWay, end = idx + numWay;
    
    int state = -1;
    for (; idx < end; idx++) {
        if (cacheLineValid(idx)) {
            if (getCacheLineTag(idx) == tag) {
                this->numHit += 1;
                setCacheLineDirty(idx);
                state = -2;
                break;
            }
        } else state = idx;
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

void Cache::setCacheLineDirty(int idx) {
    SETBIT(*((addr_t *)&this->cacheLines[idx]), DIRT_POS);
}
