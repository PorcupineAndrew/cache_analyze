#include "mem.h"

Mem::Mem(int nb, int bbw) :
    numBlock(nb), blockBitWidth(bbw) {
    this->mem = new byte[nb*bbw/sizeof(byte) + 1];
    memset(this->mem, 0, nb*bbw/sizeof(byte)+1);
}

Mem::~Mem() {
    delete [] this->mem;
}

addr_t Mem::getBlock(int idx, int _blockBitWidth) const {
    if (_blockBitWidth == -1) 
        _blockBitWidth = this->blockBitWidth;

    addr_t buf; 

    int byteStart = idx*_blockBitWidth/8;
    int byteEnd = (idx+1)*_blockBitWidth/8;
    if (((idx+1)*_blockBitWidth)%8 == 0) byteEnd -= 1;

    int numBytes = byteEnd - byteStart + 1;
    assert (numBytes <= 8);
    memcpy(&buf, &this->mem[byteStart], numBytes);

    int offset = 8 - ((byteStart+1)*8 - idx*_blockBitWidth);
    assert (offset >= 0 && offset < 8);

    return (buf >> offset) & MASK(_blockBitWidth); 
}

void Mem::setBlock(int idx, addr_t buf, int _blockBitWidth) {
    if (_blockBitWidth == -1) 
        _blockBitWidth = this->blockBitWidth;

    int byteStart = idx*_blockBitWidth/8;
    int byteEnd = (idx+1)*_blockBitWidth/8;
    if (((idx+1)*_blockBitWidth)%8 == 0) byteEnd -= 1;
    assert ((byteEnd - byteStart + 1) <= 8);

    int offset = 8 - ((byteStart+1)*8 - idx*_blockBitWidth);
    assert (offset >= 0 && offset < 8);

    (buf &= MASK(_blockBitWidth)) <<= offset;
    *((addr_t *)&this->mem[byteStart]) &= ~(MASK(_blockBitWidth) << offset);
    *((addr_t *)&this->mem[byteStart]) |= buf;
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

unsigned int Mem::getBitInBlock(int idx, int offset, int _blockBitWidth) const {
    return GETBIT(getBlock(idx, _blockBitWidth), offset);
}

void Mem::setBitInBlock(int idx, int offset, int _blockBitWidth) {
    addr_t buf = getBlock(idx, _blockBitWidth);
    setBlock(idx, SETBIT(buf, offset), _blockBitWidth);
}

void Mem::unsetBitInBlock(int idx, int offset, int _blockBitWidth) {
    addr_t buf = getBlock(idx, _blockBitWidth);
    setBlock(idx, UNSETBIT(buf, offset), _blockBitWidth);
}

int Mem::getBlockBitWidth() const {
    return this->blockBitWidth;
}
