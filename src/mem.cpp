#include "mem.h"
#include <bitset>


Mem::Mem(int nb, int bbw) :
    numBlock(nb), blockBitWidth(bbw) {
    this->mem = new byte_t[nb*bbw/sizeof(byte_t) + 1];
    memset(this->mem, 0, nb*bbw/sizeof(byte_t)+1);
}

Mem::~Mem() {
    delete [] this->mem;
}

addr_t Mem::getBlock(int idx, int _blockBitWidth) const {
    if (_blockBitWidth == -1) 
        _blockBitWidth = this->blockBitWidth;

    addr_t buf = 0; 

    int byteStart = idx*_blockBitWidth/8;
    int byteEnd = (idx+1)*_blockBitWidth/8;
    if (((idx+1)*_blockBitWidth)%8 == 0) byteEnd -= 1;

    int numBytes = byteEnd - byteStart + 1;
    int offset = 8 - ((byteStart+1)*8 - idx*_blockBitWidth);
    assert (offset >= 0 && offset < 8);
    assert (numBytes <= 9);

    if (numBytes <= 8) {
        memcpy(&buf, &this->mem[byteStart], numBytes);
        buf = (buf >> offset) & MASK(_blockBitWidth);
    } else {
        memcpy(&buf, &this->mem[byteStart], 8);
        buf >>= offset;
        *(((byte_t *)&buf) + 7) |= (this->mem[byteEnd] << (8-offset));
        buf &= MASK(_blockBitWidth);
    }

    return buf; 
}

void print_arr(byte_t *arr) {
    for (int i = 8; i>=0; i--) {
        cerr << bitset<8>(arr[i]) << " ";
    }
    cerr << endl;
}

void Mem::setBlock(int idx, addr_t buf, int _blockBitWidth) {
    if (_blockBitWidth == -1) 
        _blockBitWidth = this->blockBitWidth;

    int byteStart = idx*_blockBitWidth/8;
    int byteEnd = (idx+1)*_blockBitWidth/8;
    if (((idx+1)*_blockBitWidth)%8 == 0) byteEnd -= 1;

    int numBytes = byteEnd - byteStart + 1;
    int offset = 8 - ((byteStart+1)*8 - idx*_blockBitWidth);
    assert (offset >= 0 && offset < 8);
    assert (numBytes <= 9);

    if (numBytes <= 8) {
        (buf &= MASK(_blockBitWidth)) <<= offset;
        *((addr_t *)&this->mem[byteStart]) &= ~(MASK(_blockBitWidth) << offset);
        *((addr_t *)&this->mem[byteStart]) |= buf;
    } else {
        byte_t tmp[9];
        *((addr_t *)tmp) = buf; tmp[8] = 0;

        // cerr << "offset: " << offset << endl;
        // cerr << bitset<64>(buf) << endl;
        // cerr << "tmp:\t"; print_arr(tmp);
        // cerr << "mem:\t"; print_arr(&this->mem[byteStart]);

        *((addr_t *)(tmp+1)) <<= offset;
        tmp[1] |= (tmp[0] >> (8-offset));
        tmp[0] <<= offset;

        // cerr << "tmp:\t"; print_arr(tmp);

        tmp[0] |= (this->mem[byteStart] & MASK(offset));
        tmp[8] |= (this->mem[byteEnd] & ~MASK(offset));
        memcpy(&this->mem[byteStart], tmp, 9);

        // cerr << "tmp:\t"; print_arr(tmp);
        // cerr << "mem:\t"; print_arr(&this->mem[byteStart]);
        // cerr << endl;
    }
}

void Mem::test(int nb, int bbw) {
    Mem test(nb, bbw);
    int size = nb*bbw/8;
    addr_t* buf = new addr_t[size];
    memset(buf, 0, sizeof(addr_t)*size);
    for (int i = 0; i < size; i++) {
        buf[i] = ((((addr_t)rand()) << 32) | rand()) & MASK(bbw);
        test.setBlock(i, buf[i]);
        assert (buf[i] == test.getBlock(i));
    }
    for (int i = 0; i < size; i++) {
        assert (buf[i] == test.getBlock(i));
    }
    delete [] buf;
    cerr << "Mem IO test passed." << endl;
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
