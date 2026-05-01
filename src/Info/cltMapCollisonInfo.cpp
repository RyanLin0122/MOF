#include "Info/cltMapCollisonInfo.h"
#include <cstring>
#include <cstdlib>
#include <new>

// ============================================================================
//  cltMapCollisonInfo — mofclient.c 0x5799B0..0x579C50 還原
//  與反編譯 1:1 等價；不接觸任何 .txt（位元圖直接從 .map / 包檔載入）。
// ============================================================================

// mofclient.c 0x5799B0
//   反編譯只清零 width / height / pBits，並未寫入 rowBytes：
//     *((_DWORD *)this + 2) = 0;   // m_pBits @ +8
//     *(_WORD  *)this      = 0;   // m_wWidth @ +0
//     *((_WORD *)this + 1) = 0;   // m_wHeight @ +2
//   m_wRowBytes 在首次 Initialize* 之前是不確定值；上層所有路徑都會在
//   觀察到它之前先呼叫 Initialize* 寫入正確值，故行為等價。
cltMapCollisonInfo::cltMapCollisonInfo()
    : m_wWidth(0), m_wHeight(0), m_pBits(nullptr) {
    // m_wRowBytes 故意不寫入，保留與反編譯完全一致的構造寫入順序與寫入欄位集合。
}

// mofclient.c 0x?????? (// idb — 反編譯只列宣告，無函式體；亦即空 dtor)
//   Map::~Map() 在呼叫本 dtor 之前已先經 Map::Free 路徑釋放過 m_pBits；
//   Map 並非每次都先 Free 再析構，但所有有效呼叫鏈都保證 m_pBits 已釋放或從未配置。
cltMapCollisonInfo::~cltMapCollisonInfo() {
    // 與 ground truth 一致：dtor 為空。
}

// 計算每列位元組數：ceil(width / 8)。
//   反編譯為 (a2 & 0x80000007)，於 16-bit 隱式提升後等價於 (width & 7) != 0：
//     if ((width & 7) != 0)  rowBytes = (width >> 3) + 1;
//     else                   rowBytes =  width >> 3;
static inline unsigned short ComputeRowBytes(unsigned short width) {
    return static_cast<unsigned short>((width & 7) ? ((width >> 3) + 1) : (width >> 3));
}

// mofclient.c 0x5799D0
//   寫入順序與反編譯一致：width → height → rowBytes → operator new → memset(0) →
//   memcpy(total)。原始為 qmemcpy 4-byte 區塊 + 尾端 (total & 3)；語意上等於
//   memcpy(dst, src, total)，總拷貝長度 = height * rowBytes。
int cltMapCollisonInfo::InitializeInPack(unsigned short width, unsigned short height, char* src) {
    m_wWidth    = width;
    m_wHeight   = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    std::memcpy(m_pBits, src, total);
    return 1;
}

// mofclient.c 0x579A70
//   寫入順序：width → height → rowBytes → operator new → memset(0) → fread(total)。
//   注意 fread 的 size/count 順序：原始為 fread(buf, 1u, total, stream)。
int cltMapCollisonInfo::Initialize(unsigned short width, unsigned short height, FILE* stream) {
    m_wWidth    = width;
    m_wHeight   = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    std::fread(m_pBits, 1, total, stream);
    return 1;
}

// mofclient.c 0x579B10
//   僅配置並清零，不讀入資料。版本欄位 != 11 時走此路徑。
int cltMapCollisonInfo::Initialize(unsigned short width, unsigned short height) {
    m_wWidth    = width;
    m_wHeight   = height;
    m_wRowBytes = ComputeRowBytes(width);

    const std::size_t total = static_cast<std::size_t>(height) * m_wRowBytes;
    m_pBits = static_cast<unsigned char*>(operator new(total));
    std::memset(m_pBits, 0, total);
    return 1;
}

// mofclient.c 0x579B90（與 InitializeInPack 機械碼幾乎一致，獨立位址）
int cltMapCollisonInfo::Initialize(unsigned short width, unsigned short height, char* src) {
    m_wWidth    = width;
    m_wHeight   = height;
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
//   越界 → 1；位元為 1 → 1；其餘 → 0。switch 分支與反編譯逐 case 對應。
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
