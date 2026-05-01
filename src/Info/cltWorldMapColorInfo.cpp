#include "Info/cltWorldMapColorInfo.h"
#include "FileSystem/CMOFPacking.h"
#include <cstdio>
#include <cstring>
#include <new>

// ----- (00503320 — mofclient.c:213837) -----------------------------------
// 反編譯：*(_DWORD *)this = 0;
cltWorldMapColorInfo::cltWorldMapColorInfo()
    : m_pData(nullptr) {}

cltWorldMapColorInfo::~cltWorldMapColorInfo() {
    Free();
}

// ----- (00503340 — mofclient.c:213847) -----------------------------------
// 反編譯流程（逐步對照）：
//   1. char v7[256]; strcpy(v7, a2);
//   2. v3 = CMofPacking::ChangeString(&g_clMofPacking, v7);
//   3. CMofPacking::FileReadBackGroundLoading(&g_clMofPacking, v3);
//   4. if (!&dword_C24CF4) return 0;          // 對全域陣列取址，恆真：實際是死碼
//   5. v5 = operator new(0x210D3u);           // throwing new
//      *(_DWORD *)this = v5;
//   6. qmemcpy(v5, &dword_C24CF4, 0x210D0u);  // 0x210D0 bytes
//      v6 = v5 + 67688;                       // (_WORD*)+67688 = (_BYTE*)+0x210D0
//      v5[67688] = *((_WORD *)&dword_C24CF4 + 67688);  // 2 bytes @ +0x210D0
//      *((_BYTE *)v6 + 2) = *((_BYTE *)&dword_C24CF4 + 135378); // 1 byte @ +0x210D2
//   7. return 1;
int cltWorldMapColorInfo::InitializeInPack(char* pathInPack) {
    char localPath[256];
    std::strcpy(localPath, pathInPack);

    CMofPacking* pack = CMofPacking::GetInstance();
    char* changed = pack->ChangeString(localPath);
    pack->FileReadBackGroundLoading(changed);

    // 對應 dword_C24CF4：背景載入後資料起始於 m_backgroundLoadBufferField[0]
    const uint8_t* src =
        reinterpret_cast<const uint8_t*>(pack->m_backgroundLoadBufferField);

    // 與反編譯一致：throwing operator new、無條件指派 m_pData
    m_pData = static_cast<uint8_t*>(::operator new(kSize));

    // qmemcpy(0x210D0) + WORD@0x210D0 + BYTE@0x210D2 = 0x210D3 bytes
    std::memcpy(m_pData, src, 0x210D0);
    std::memcpy(m_pData + 0x210D0, src + 0x210D0, 2);  // WORD copy
    m_pData[0x210D2] = src[0x210D2];                   // BYTE copy

    return 1;
}

// ----- (005033E0 — mofclient.c:213872) -----------------------------------
// 反編譯：
//   v3 = 0;
//   v4 = fopen(FileName, "rb");
//   if (v4) {
//     v5 = operator new(0x210D3u);
//     *(_DWORD *)this = v5;
//     if (fread(v5, 0x210D3u, 1u, v4) == 1) v3 = 1;
//     fclose(v4);
//   }
//   return v3;
//
// 注意：fread 失敗時，反編譯並未釋放 v5；m_pData 仍指向已配置的 buffer，
//       由後續 Free() 統一回收（保留物件「持有資料」的不變式）。
int cltWorldMapColorInfo::Initialize(const char* fileName) {
    int v3 = 0;
    std::FILE* fp = std::fopen(fileName, "rb");
    if (fp) {
        // 與反編譯一致：無條件配置並指派；fread 失敗也不釋放
        m_pData = static_cast<uint8_t*>(::operator new(kSize));
        if (std::fread(m_pData, kSize, 1u, fp) == 1u) {
            v3 = 1;
        }
        std::fclose(fp);
    }
    return v3;
}

// ----- (00503440 — mofclient.c:213892) -----------------------------------
void cltWorldMapColorInfo::Free() {
    if (m_pData) {
        ::operator delete(m_pData);
        m_pData = nullptr;
    }
}

// ----- (00503460 — mofclient.c:213902) -----------------------------------
// 反編譯：
//   if (*(_DWORD *)this && a2 >= 0 && a3 >= 0 && a2 < 409 && a3 < 331)
//     result = *(uint8_t*)(a3 + *(_DWORD *)this + 408*a3 + a2);
//   else
//     result = 0;
//
// 位移計算： a3 + 408*a3 + a2  ==  409*y + x
int cltWorldMapColorInfo::GetColor(int x, int y) const {
    if (m_pData && x >= 0 && y >= 0 && x < kWidth && y < kHeight) {
        size_t offset = static_cast<size_t>(y) * static_cast<size_t>(kWidth)
                      + static_cast<size_t>(x);
        return static_cast<int>(m_pData[offset]);
    }
    return 0;
}
