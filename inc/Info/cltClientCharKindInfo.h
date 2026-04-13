#pragma once
#include <cstdint>
#include "Info/cltCharKindInfo.h"
#include "Info/cltMonsterAniInfo.h"

// cltClientCharKindInfo 還原自 mofclient.c (0x00401350 .. 0x004015F0)。
//
// Layout (來自反編譯):
//   offset 0x00           : vftable
//   offset 0x04 .. 0x3FFFF: 65535 stCharKindInfo*  (繼承自 cltCharKindInfo)
//   offset 0x40000        : 其他 cltCharKindInfo 成員 (monster name buffer...)
//   offset 0x40008        : 65535 cltMonsterAniInfo* (本類別新增)
//
// 在此 C++ 還原中，父類別的 slot 陣列改為 heap-allocated，以減少 BSS；
// cltMonsterAniInfo 指標表同樣改為 heap-allocated。行為與反編譯完全等價。
class cltClientCharKindInfo : public cltCharKindInfo {
public:
    cltClientCharKindInfo();
    virtual ~cltClientCharKindInfo() override;

    // Initialize / Free 覆蓋父類別虛擬方法 (vftable slot 1, 2)
    virtual int Initialize(char* String2) override;
    virtual void Free() override;

    // 取得指定 char kind 的 cltMonsterAniInfo*（惰性載入）
    cltMonsterAniInfo* GetMonsterAniInfo(unsigned short kindCode);

    // 取得指定 char kind 的等級 (char kind info offset 146, BYTE)
    unsigned char GetCharLevel(unsigned short kindCode);

    // 回傳 (void*) — 對齊反編譯：若 IsFieldItemBox 旗標設定則非 null。
    // 原始 binary 從 char kind info offset 236 (DWORD) 讀取。
    void* IsFieldItemBox(unsigned short kindCode);

private:
    // 65535 pointer slots；對齊反編譯 (char*)this + 0x40008 開始之資料。
    cltMonsterAniInfo** m_ppMonsterAniInfoTable;
};
