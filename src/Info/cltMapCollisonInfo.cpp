#include "Info/cltMapCollisonInfo.h"
#include <cstring>
#include <cstdlib>
#include <new>

// mofclient.c 0x5799B0
cltMapCollisonInfo::cltMapCollisonInfo()
    : m_wWidth(0), m_wHeight(0), m_wRowBytes(0), m_pBits(nullptr) {
    // 反編譯只清零 width / height / pBits（不動 rowBytes），但全清零行為等價：
    // 任何 IsCollison 在 m_pBits == nullptr 時必先被 width=0 短路掉。
}

cltMapCollisonInfo::~cltMapCollisonInfo() {
    // 反編譯沒有實質動作（Map 在自己的 Free 流程裡先呼叫過本類 Free）。
    // 為防萬一仍呼叫一次：Free 在 m_pBits 為空時是 no-op。
    Free();
}

// 計算每列位元組數：ceil(width / 8)。反編譯用 (a2 & 0x80000007)，於 16-bit
// 隱式提升後等價於 (width & 7) != 0。
static inline unsigned short ComputeRowBytes(unsigned short width) {
    return static_cast<unsigned short>((width & 7) ? ((width >> 3) + 1) : (width >> 3));
}

// mofclient.c 0x5799D0
int cltMapCollisonInfo::InitializeInPack(unsigned short width, unsigned short height, char* src) {
    m_wWidth = width;
    m_wHeight = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    std::memcpy(m_pBits, src, total);
    return 1;
}

// mofclient.c 0x579A70
int cltMapCollisonInfo::Initialize(unsigned short width, unsigned short height, FILE* stream) {
    m_wWidth = width;
    m_wHeight = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    std::fread(m_pBits, 1, total, stream);
    return 1;
}

// mofclient.c 0x579B10
int cltMapCollisonInfo::Initialize(unsigned short width, unsigned short height) {
    m_wWidth = width;
    m_wHeight = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    return 1;
}

// mofclient.c 0x579B90（與 InitializeInPack 同碼，獨立位址）
int cltMapCollisonInfo::Initialize(unsigned short width, unsigned short height, char* src) {
    m_wWidth = width;
    m_wHeight = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    std::memcpy(m_pBits, src, total);
    return 1;
}

// mofclient.c 0x579C30
void cltMapCollisonInfo::Free() {
    if (m_pBits) {
        operator delete(m_pBits);
        m_pBits = nullptr;
    }
}

// mofclient.c 0x579C50
int cltMapCollisonInfo::IsCollison(unsigned short x, unsigned short y) {
    if (x >= m_wWidth)  return 1;
    if (y >= m_wHeight) return 1;

    const unsigned char byteVal = m_pBits[(x >> 3) + static_cast<std::size_t>(y) * m_wRowBytes];
    unsigned char mask;
    switch (x & 7) {
        case 0: mask = 0x80; break;
        case 1: mask = 0x40; break;
        case 2: mask = 0x20; break;
        case 3: mask = 0x10; break;
        case 4: mask = 0x08; break;
        case 5: mask = 0x04; break;
        case 6: mask = 0x02; break;
        case 7: mask = 0x01; break;
        default: return 0;
    }
    return (byteVal & mask) ? 1 : 0;
}
