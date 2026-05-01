#pragma once
#include <cstdint>
#include "Info/cltPetKindInfo.h"
#include "Info/cltPetAniInfo.h"

// ---------------------------------------------------------------------------
// cltClientPetKindInfo — Ground truth: mofclient.c
//   - cltClientPetKindInfo::cltClientPetKindInfo @ 004EDA20
//   - cltClientPetKindInfo::~cltClientPetKindInfo@ 004EDA60
//   - cltClientPetKindInfo::Free                 @ 004EDAC0
//   - cltClientPetKindInfo::GetPetAniInfo        @ 004EDAF0
//   - cltClientPetKindInfo::GetPetUIImage        @ 004EDBE0
//
// 32-bit Memory Layout (反編譯):
//   offset 0x00: vftable (4 bytes)                                      // KR: 가상함수 테이블 / CN: 虛擬函式表
//   offset 0x04: cltPetKindInfo m_petKindInfo (16 bytes embedded)       // KR: 펫 종류 정보   / CN: 寵物種類資料
//   offset 0x14: cltPetAniInfo* m_aniInfoTable[0xFFFF] (animation lazy cache)
//                65535 個指標，惰性載入（GetPetAniInfo 內 new + Initialize）
//                BSS 零值初始化 (g_clClientPetKindInfo 是全域物件)
//
// 64-bit build 因指標寬度 8 bytes，記憶體佈局擴張：
//   offset 0x00: vptr (8)
//   offset 0x08: m_petKindInfo (24 bytes 在 64-bit 對齊下)
//   offset 0x20: m_aniInfoTable[0xFFFF] (約 524280 bytes)
// 邏輯行為與 32-bit ground truth 完全一致；外部僅透過成員函式存取。
// ---------------------------------------------------------------------------
class cltClientPetKindInfo {
public:
    cltClientPetKindInfo();
    virtual ~cltClientPetKindInfo();

    // 釋放所有 lazy 配置的動畫資訊 (對應 sub_004EDAC0)
    void Free();

    // 取得（惰性載入）指定 pet kind 的動畫資訊 (對應 sub_004EDAF0)
    cltPetAniInfo* GetPetAniInfo(uint16_t petKind);

    // 取得 pet UI 圖示 (對應 sub_004EDBE0)
    //   *outResId   ← strPetKindInfo::dwPetStopResource (offset 148)
    //   *outFrame   ← strPetKindInfo::wPetStopBlockId   (offset 152)
    void GetPetUIImage(uint16_t petKind, unsigned int* outResId, uint16_t* outFrame);

    // 直接存取嵌入的 pet kind info（供 cltPetSystem / cltPetKeepingSystem 等讀取）
    cltPetKindInfo&       PetKindInfo()       { return m_petKindInfo; }
    const cltPetKindInfo& PetKindInfo() const { return m_petKindInfo; }

private:
    // KR: 펫 종류 정보 (composition)         CN: 寵物種類資料(組合)
    // 32-bit ground truth offset = 0x04 (vftable 之後)
    cltPetKindInfo  m_petKindInfo;

    // KR: 펫 애니메이션 정보 캐시 (lazy)     CN: 寵物動畫資訊快取(惰性)
    // 32-bit ground truth offset = 0x14；BSS 零值初始化（依賴全域物件特性）
    cltPetAniInfo*  m_aniInfoTable[0xFFFF];
};
