#include "cache.h"

Cache::Cache(int ts, int bs, int nw, cache_strategy_t st) : 
    numBitTotoalSize(ts), numBitBlockSize(bs), numBitWay(nw), numBitGroup(ts-bs-nw),
    numCacheLine(POWER2(ts-bs)), numBitOffset(numBitBlockSize), numBitIndex(numBitGroup),
    numBitTag(ADDR_BIT-numBitIndex-numBitOffset), validBit(numBitTag+VALID_OFFSET),
    dirtyBit(numBitTag+DIRTY_OFFSET), strategy(st), numHit(0), numAccess(0), numPurge(0) {

    cerr << "Cache init\n"
         << "--------------------------------\n"
         << "total size  : " << POWER2(this->numBitTotoalSize) << "\n"
         << "block size  : " << POWER2(this->numBitBlockSize) << "\n"
         << "way number  : " << POWER2(this->numBitWay) << "\n"
         << "group number: " << POWER2(this->numBitGroup) << "\n"
         << "cache lines : " << this->numCacheLine << "\n"
         << "write back  : " << isWriteBack() << "\n"
         << "write direct: " << isWriteDirect() << "\n"
         << "write assign: " << isWriteAssign() << "\n"
         << "replace algo: " << (isLRU() ? "LRU" : (isRAND() ? "RAND" : "TREE")) << "\n"
         << "--------------------------------" << endl;

    this->cacheLines = new Mem(this->numCacheLine, 2+this->numBitTag);

    const int numWay = POWER2(this->numBitWay), numGroup = POWER2(this->numBitGroup);
    if (isLRU())
        this->lruRecords = new Mem(numGroup, this->numBitWay*numWay); // TODO
    if (isTREE())
        this->treeRecords = new Mem(numGroup, numWay-1); // TODO
}

Cache::~Cache() {
    delete this->cacheLines;
    if (this->lruRecords) delete this->lruRecords;
    if (this->treeRecords) delete this->treeRecords;
}

void Cache::updateCacheLine(int idx, addr_t addr) {
    addr_t buf = SLICE(addr, this->numBitOffset+this->numBitIndex, ADDR_BIT);
    SETBIT(buf, this->validBit);
    this->cacheLines->setBlock(idx, buf);
}

bool Cache::cacheLineValid(int idx) const {
    return this->cacheLines->getBitInBlock(idx, this->validBit);
}

bool Cache::cacheLineDirty(int idx) const {
    return this->cacheLines->getBitInBlock(idx, this->dirtyBit);
}

addr_t Cache::getCacheLineTag(int idx) {
    return SLICE(this->cacheLines->getBlock(idx), 0, this->numBitTag);
}

int Cache::read(addr_t addr) {
    this->numAccess += 1;
    const int numWay = POWER2(this->numBitWay);

    unsigned int index = SLICE(addr, this->numBitOffset, this->numBitOffset+this->numBitIndex);
    index <<= this->numBitWay;
    addr_t tag = SLICE(addr, this->numBitOffset+this->numBitIndex, ADDR_BIT);

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
            if (isLRU()) updateLRU(index+state);
            else if (isTREE()) updateTREE(index+state);
            updateCacheLine(index+state, addr);
        } else {
            int idx = purge(index);
            updateCacheLine(idx, addr);
        }
    }
    return state == -2;
}

int Cache::write(addr_t addr) {
    this->numAccess += 1;
    const int numWay = POWER2(this->numBitWay);

    unsigned int index = SLICE(addr, this->numBitOffset, this->numBitOffset+this->numBitIndex);
    index <<= this->numBitWay;
    addr_t tag = SLICE(addr, this->numBitOffset+this->numBitIndex, ADDR_BIT);
    
    int state = -1;
    for (int i = 0; i < numWay; i++) {
        if (cacheLineValid(index+i)) {
            if (getCacheLineTag(index+i) == tag) {
                hit(index+i);
                setCacheLineDirty(index+i);
                state = -2;
                break;
            }
        } else state = i;
    }
    if (state != -2) {
        if (isWriteAssign()) {
            if (state >= 0) {
                if (isLRU()) updateLRU(index+state);
                else if (isTREE()) updateTREE(index+state);
                updateCacheLine(index+state, addr);
            } else {
                int idx = purge(index);
                updateCacheLine(idx, addr);
            }
        }
    }
    return state == -2;
}

void Cache::setCacheLineValid(int idx) {
    this->cacheLines->setBitInBlock(idx, this->validBit);
}

void Cache::unsetCacheLineValid(int idx) {
    this->cacheLines->unsetBitInBlock(idx, this->validBit);
}

void Cache::setCacheLineDirty(int idx) {
    this->cacheLines->setBitInBlock(idx, this->dirtyBit);
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
        updateTREE(idx);
    }
}

int Cache::purge(int index) {
    this->numPurge += 1;
    if (isLRU()) {
        const int numWay = POWER2(this->numBitWay);

        int idx = -1, max = 0;
        for (int i = 0; i < numWay; i++) {
            assert (cacheLineValid(index+i));
            int rank = LRU_RANK(index+i);
            if (rank > max) {
                max = rank;
                idx = index+i;
            }
        }

        assert (max+1 == numWay);
        updateLRU(idx);
        return idx;
    } else if (isRAND()) {
        return index + (rand() & MASK(this->numBitWay));
    } else if (isTREE()) {
        const int numWay = POWER2(this->numBitWay), leafOffset = numWay - 1,
                treeIndex = index - (index >> this->numBitWay);

        int node = 0;
        while (node < leafOffset) {
            node = (node << 1) + 1 + (TREE_MARK(node+treeIndex) == LEFT ? LEFT : RIGHT);
        }

        int idx = node - leafOffset + index;
        updateTREE(idx);
        return idx;
    } else {
        cerr << "Error on repalce" << endl;
        exit(0);
    }
}

void Cache::updateLRU(int idx) {
    const int index = (idx >> this->numBitWay) << this->numBitWay,
                numWay = POWER2(this->numBitWay);
    unsigned int upperRank = cacheLineValid(idx) ? 
                                LRU_RANK(idx) : (1U<<this->numBitWay);

    for (int i = 0; i < numWay; i++) {
        if (index+i == idx) {
            SET_LRU_RANK(idx, 0);
            continue;
        }
        addr_t buf = LRU_RANK(index+i);
        if (cacheLineValid(index+i) && (buf < upperRank)) {
            SET_LRU_RANK(index+i, buf+1);
        }
    }
}

void Cache::updateTREE(int idx) {
    const int index = (idx >> this->numBitWay) << this->numBitWay,
            numWay = POWER2(this->numBitWay),
            leafOffset = numWay - 1,
            treeIndex = index - (index >> this->numBitWay);

    idx = idx - index + leafOffset;
    do {
        if ((idx-1)%2 == LEFT) SET_TREE_RIGHT(treeIndex+(idx-1)/2);
        else SET_TREE_LEFT(treeIndex+(idx-1)/2);
    } while ((idx=(idx-1)/2));
}

void Cache::print() {
    cerr << "total access: " << this->numAccess << "\n"
         << "hit times   : " << this->numHit << "\n"
         << "purge times : " << this->numPurge << "\n"
         << "miss ratio  : " << 1.0 - (this->numHit + 0.0) / this->numAccess << "\n"
         << "--------------------------------" << endl;
}
