#include "Info/cltWorldMapColorInfo.h"
#include "FileSystem/CMOFPacking.h"  // 使用單例與背景載入緩衝
#include <cstdio>
#include <cstring>
#include <new>

// ----- (00503320) --------------------------------------------------------
cltWorldMapColorInfo::cltWorldMapColorInfo()
    : m_pData(nullptr) {}

// 以 operator delete 對應釋放，避免遺漏
cltWorldMapColorInfo::~cltWorldMapColorInfo() {
    Free();
}

// ----- (00503340) --------------------------------------------------------
// InitializeInPack: 透過封包系統讀入，並複製 0x210D3 bytes 到內部緩衝
int cltWorldMapColorInfo::InitializeInPack(char* pathInPack) {
    // 反編譯碼使用本地 256 bytes 緩衝後再 ChangeString
    char localPath[256];
    std::strcpy(localPath, pathInPack);

    CMofPacking* pack = CMofPacking::GetInstance();
    if (!pack) return 0;

    char* changed = pack->ChangeString(localPath);
    pack->FileReadBackGroundLoading(changed);

    // 原碼以 &dword_C24CF4 判斷；此處對應到單例內的背景緩衝區
    const uint8_t* src = reinterpret_cast<const uint8_t*>(pack->m_backgroundLoadBufferField);
    if (!src) return 0;

    // 配置與反編譯一致大小（operator new / operator delete 配對）
    void* p = ::operator new(kSize, std::nothrow);
    if (!p) return 0;
    m_pData = static_cast<uint8_t*>(p);

    // 反編譯中先 qmemcpy 0x210D0，再補 2 bytes 的 WORD 與 1 byte：
    //   v5[67688] 與 *((_BYTE*)v6 + 2) 對應位移 0x210D0、0x210D1、0x210D2
    std::memcpy(m_pData, src, 0x210D0);
    m_pData[0x210D0] = src[0x210D0];
    m_pData[0x210D1] = src[0x210D1];
    m_pData[0x210D2] = src[0x210D2];

    return 1;
}

// ----- (005033E0) --------------------------------------------------------
// Initialize: 直接從檔案讀入 0x210D3 bytes
int cltWorldMapColorInfo::Initialize(const char* fileName) {
    int ok = 0;
    std::FILE* fp = std::fopen(fileName, "rb");
    if (fp) {
        void* p = ::operator new(kSize, std::nothrow);
        if (p) {
            m_pData = static_cast<uint8_t*>(p);
            if (std::fread(m_pData, kSize, 1u, fp) == 1u) {
                ok = 1;
            }
            else {
                ::operator delete(m_pData);
                m_pData = nullptr;
            }
        }
        std::fclose(fp);
    }
    return ok;
}

// ----- (00503440) --------------------------------------------------------
void cltWorldMapColorInfo::Free() {
    if (m_pData) {
        ::operator delete(m_pData);
        m_pData = nullptr;
    }
}

// ----- (00503460) --------------------------------------------------------
// GetColor: 若已載入且座標在 [0,409)×[0,331) 內，回傳位於 base + (409*y + x) 的 uint8 值。
int cltWorldMapColorInfo::GetColor(int x, int y) const {
    if (m_pData && x >= 0 && y >= 0 && x < kWidth && y < kHeight) {
        // 反編譯位移：*(base + a2 + 408*a3 + a3) == base + (409*y + x)
        size_t offset = static_cast<size_t>(y) * static_cast<size_t>(kWidth) + static_cast<size_t>(x);
        return static_cast<int>(m_pData[offset]);
    }
    return 0;
}
