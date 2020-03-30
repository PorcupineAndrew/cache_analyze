#ifndef _MACRO_H_
#define _MACRO_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string.h>

#define POWER2(x) (1ULL << (x))

#define STREAM_ADDR(x) \
    std::setfill('0') << std::setw(sizeof((x))*2) << std::hex << (x)

#define BITSIZEOF(x) (sizeof(x) << 3)

#define SLICE(x, l, h) \
    (((x) << (BITSIZEOF(x) - (h))) >> (BITSIZEOF(x) - (h) + (l)))

#define SETBIT(x, pos) ((x) |= (1ULL << (pos)))
#define UNSETBIT(x, pos) ((x) &= (~(1ULL << (pos))))

#define ADDR_BIT     64

#define VALID_POS    (BITSIZEOF(cache_line_t)-1)
#define DIRT_POS     (BITSIZEOF(cache_line_t)-2)

typedef unsigned long long int addr_t;
typedef unsigned char cache_line_t[7];

using namespace std;

#endif
