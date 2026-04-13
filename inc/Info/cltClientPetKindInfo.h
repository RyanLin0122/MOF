#pragma once
#include <cstdint>
#include "Info/cltPetKindInfo.h"
#include "Info/cltPetAniInfo.h"

// cltClientPetKindInfo 還原自 mofclient.c (0x004EDA20 .. 0x004EDBE0)
//
// Layout (來自反編譯):
//   offset 0x00: vftable
//   offset 0x04: cltPetKindInfo (嵌入成員)
//   offset 0x14: 65535 個 cltPetAniInfo*  (動畫資訊快取)
//
// 此 C++ 還原採用組合（composition），與反編譯行為等價：
// cltPetKindInfo 為成員，在建構子被初始化；動畫表亦為固定陣列成員。
class cltClientPetKindInfo {
public:
    cltClientPetKindInfo();
    virtual ~cltClientPetKindInfo();

    // 釋放所有已配置的動畫資訊 (對應反編譯 cltClientPetKindInfo::Free)
    void Free();

    // 取得（惰性載入）指定 pet kind 的動畫資訊
    cltPetAniInfo* GetPetAniInfo(uint16_t petKind);

    // 取得 pet UI 圖示（資源 ID、block frame）
    // a3 = offset 148 (dwPetStopResource)
    // a4 = offset 152 (wPetStopBlockId)
    void GetPetUIImage(uint16_t petKind, unsigned int* outResId, uint16_t* outFrame);

    // 直接存取 pet kind info 表（供外部系統讀取資料）
    cltPetKindInfo& PetKindInfo() { return m_petKindInfo; }
    const cltPetKindInfo& PetKindInfo() const { return m_petKindInfo; }

private:
    cltPetKindInfo  m_petKindInfo;                 // +0x04 (after vftable)
    cltPetAniInfo*  m_aniInfoTable[0xFFFF] = {};   // +0x14
};
