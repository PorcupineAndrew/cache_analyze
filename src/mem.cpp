#include "mem.h"

Mem::Mem(int nb, int bbw) :
    numBlock(nb), blockBitWidth(bbw) {
    this->mem = new byte[nb*bbw/sizeof(byte) + 1];
    memset(this->mem, 0, nb*bbw/sizeof(byte)+1);

    // cerr << "Mem init\n"
    //      << "--------------------------------\n"
    //      << "numBlock       : " << this->numBlock << "\n"
    //      << "blockBitWidth  : " << this->blockBitWidth << "\n"
    //      << "--------------------------------" << endl;
}

Mem::~Mem() {
    delete [] this->mem;
}

addr_t Mem::getBlock(int idx) const {
    addr_t buf; 

    int byteStart = idx*this->blockBitWidth/8;
    int byteEnd = (idx+1)*this->blockBitWidth/8;
    if (((idx+1)*this->blockBitWidth)%8 == 0) byteEnd -= 1;

    int numBytes = byteEnd - byteStart + 1;
    assert (numBytes <= 8);
    memcpy(&buf, &this->mem[byteStart], numBytes);

    int offset = 8 - ((byteStart+1)*8 - idx*this->blockBitWidth);
    assert (offset >= 0 && offset < 8);

    // cerr << "get idx    : " << idx << "\n"
    //      << "byteStart  : " << byteStart << "\n"
    //      << "byteEnd    : " << byteEnd << "\n"
    //      << "numBytes   : " << numBytes << "\n"
    //      << "offset     : " << offset << "\n"
    //      << "--------------------------------" << endl;
    return (buf >> offset) & MASK(this->blockBitWidth); 
}

void Mem::setBlock(int idx, addr_t buf) {
    int byteStart = idx*this->blockBitWidth/8;
    int byteEnd = (idx+1)*this->blockBitWidth/8;
    if (((idx+1)*this->blockBitWidth)%8 == 0) byteEnd -= 1;

    int offset = 8 - ((byteStart+1)*8 - idx*this->blockBitWidth);

    (buf &= MASK(this->blockBitWidth)) <<= offset;
    *((addr_t *)&this->mem[byteStart]) &= ~(MASK(this->blockBitWidth) << offset);
    *((addr_t *)&this->mem[byteStart]) |= buf;

    // cerr << "set idx    : " << idx << "\n"
    //      << "byteStart  : " << byteStart << "\n"
    //      << "byteEnd    : " << byteEnd << "\n"
    //      << "offset     : " << offset << "\n"
    //      << "--------------------------------" << endl;
}

void Mem::test(int nb, int bbw) {
    Mem test(nb, bbw);
    for (int i = 0; i < nb; i++) {
        addr_t buf = i;
        test.setBlock(i, buf);
    }
    for (int i = 0; i < nb; i++) {
        assert (((addr_t)i) == test.getBlock(i));
    }
}
