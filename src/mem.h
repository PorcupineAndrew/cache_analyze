#ifndef _MEM_H_
#define _MEM_H_
#include "macro.h"

class Mem {
private:
    byte_t *mem;
    const int numBlock;
    const int blockBitWidth;

public:
    Mem(int nb, int bbw);
    ~Mem();

    addr_t getBlock(int idx, int _blockBitWidth=-1) const;
    void setBlock(int idx, addr_t buf, int _blockBitWidth=-1);

    unsigned int getBitInBlock(int idx, int offset, int _blockBitWidth=-1) const;
    void setBitInBlock(int idx, int offset, int _blockBitWidth=-1);
    void unsetBitInBlock(int idx, int offset, int _blockBitWidth=-1);

    int getBlockBitWidth() const;
public:
    static void test(int nb, int bbw);
};

#endif
