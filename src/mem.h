#ifndef _MEM_H_
#define _MEM_H_
#include "macro.h"

class Mem {
private:
    byte *mem;
    const int numBlock;
    const int blockBitWidth;

public:
    Mem(int nb, int bbw);
    ~Mem();

    addr_t getBlock(int idx) const;
    void setBlock(int idx, addr_t buf);

    static void test(int nb, int bbw);
};

#endif

