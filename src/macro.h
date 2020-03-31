#ifndef _MACRO_H_
#define _MACRO_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <cstdlib>
#include <cassert>

#define POWER2(x) (1ULL << (x))

#define STREAM_ADDR(x) \
    std::setfill('0') << std::setw(sizeof((x))*2) << std::hex << (x)

#define BITSIZEOF(x) (sizeof(x) << 3)

#define SLICE(x, l, h) \
    (((x) << (BITSIZEOF(x) - (h))) >> (BITSIZEOF(x) - (h) + (l)))

#define SETBIT(x, pos) ((x) |= (1ULL << (pos)))
#define UNSETBIT(x, pos) ((x) &= (~(1ULL << (pos))))
#define GETBIT(x, pos) (((x) & (1ULL << (pos))) >> (pos))
#define MASK(bit) ((1U << (bit)) - 1)

#define ADDR_BIT     64

#define VALID_POS    (BITSIZEOF(cache_line_t)-1)
#define DIRT_POS     (BITSIZEOF(cache_line_t)-2)

#define REPLACE_LRU         0U
#define REPLACE_RAND        1U
#define REPLACE_TREE        2U

#define WRITE_ASSIGN        1U
#define WRITE_NO_ASSIGN     0U

#define WRITE_BACK          1U
#define WRITE_DIRECT        0U

#define WRITE_ASSIGN_POS    0
#define WRITE_BACK_POS      1
#define REPLACE_POS         2

#define GET_STRATEGY(replace, writeAssign, writeBack) \
    ((cache_strategy_t)(((replace) << REPLACE_POS)) | ((writeBack) << WRITE_BACK_POS) | ((writeAssign) << WRITE_ASSIGN_POS))

typedef unsigned long long int addr_t;
typedef unsigned char cache_line_t[7];
typedef unsigned char cache_strategy_t;
typedef unsigned char lru_record_t[3];
typedef unsigned char tree_record_t;

using namespace std;

#endif
