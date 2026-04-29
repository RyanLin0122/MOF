#pragma once

// =============================================================================
// cltMoFC_EquipmentInfo
//
// 還原自 mofclient.c 0x4FF800 起（行 211002~）。在原 binary 是一個 polymorphic
// 類別，內嵌兩組指標陣列（offset +4 起 8 格、offset +36 起 14 格）以及 21 個
// WORD 欄位（offset +92 起，索引 46..66），用來把所有可裝備物品依 ItemType
// 分類成類別專屬的清單，提供 GUI / 任務 / 制造系統查詢。
//
// 完整 Init 會遍歷整個 cltItemKindInfo DB（最多 65535 筆）把與目前
// classKindInfo 對得上的武器 / 防具 ItemKindInfo 指標收集到對應的 bucket，
// 涵蓋 cltClassKindInfo / cltItemKindInfo 內部欄位語意；遠超出 cltMyCharData
// 還原所需的範圍。
//
// 本還原檔提供「介面 + no-op 實作」的 stub，讓 cltMyCharData::Initialize 與
// cltMyCharData 解構流程能呼叫到對應方法，不會 link error 也不會產生副作用。
// 進一步還原時將 ItemKind/ClassKind 的 byte-offset 欄位都先具名後再恢復本體。
// =============================================================================

#include <cstdint>

struct stItemKindInfo;

class cltMoFC_EquipmentInfo {
public:
    cltMoFC_EquipmentInfo();
    virtual ~cltMoFC_EquipmentInfo();

    void Free();
    void Init(std::uint16_t classKind);

    void WeaponItemCnt(stItemKindInfo* info);
    void WeaponItemInit(stItemKindInfo* info);

    std::uint16_t GetEquipItemListPtr(std::uint16_t a2,
                                      stItemKindInfo** a3,
                                      stItemKindInfo*** a4,
                                      std::uint16_t* a5,
                                      std::uint16_t* a6);
};

extern cltMoFC_EquipmentInfo g_clMoFC_EquipmentInfo;
