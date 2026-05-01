// ============================================================================
// cltMoFC_EquipmentInfo  ── mofclient.c 0x4FF8B0..0x500480 (line 211002~211591)
//
// 7 個方法逐一還原；每段流程結尾標出對應 GT 行號，便於 diff 校對。
// 直接參照 g_clItemKindInfo / g_clClassKindInfo（皆已 Initialize 完畢）。
// ============================================================================

#include "Info/cltMoFC_EquipmentInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltClassKindInfo.h"
#include "global.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

// ----------------------------------------------------------------------------
// 內部工具：與 GT 同一個 file-static qsort 比較器
//   GT 0x517690 (mofclient.c:210988)  int compareItemLevel(a1, a2)
//   依 *((stItemKindInfo*)*a1)+120 (Equip.Hunt.m_byLevel) 升冪 - clamp to ±1。
// ----------------------------------------------------------------------------
namespace {

int __cdecl compareItemLevel(const void* a1, const void* a2) {
    const stItemKindInfo* p1 = *static_cast<const stItemKindInfo* const*>(a1);
    const stItemKindInfo* p2 = *static_cast<const stItemKindInfo* const*>(a2);
    int diff = static_cast<int>(reinterpret_cast<const std::uint8_t*>(p1)[120])
             - static_cast<int>(reinterpret_cast<const std::uint8_t*>(p2)[120]);
    if (diff == 0) return 0;
    return diff < 0 ? -1 : 1;
}

// 取得 stItemKindInfo 內職業限制 64-bit bitmask。
// GT 用 *((_QWORD *)v4 + 14) 直接讀 byte offset 112；對應到還原結構即
// Equip.Hunt.m_szEquipableClass[4..11]（前 4 byte 為佔位 padding）。
inline std::uint64_t GetItemClassAtb(const stItemKindInfo* item) {
    std::uint64_t atb = 0;
    std::memcpy(&atb, &item->Equip.Hunt.m_szEquipableClass[4], sizeof(atb));
    return atb;
}

// equip-bucket 索引常數（與 m_ppLists / m_wCounts 對齊）
constexpr int IDX_CAP      = 0;  // 0x1000   1<<12  → +46/+1
constexpr int IDX_RING     = 1;  // 0x2000   1<<13  → +47/+2
constexpr int IDX_NECKLACE = 2;  // 0x4000   1<<14  → +48/+3
constexpr int IDX_SHIELD   = 3;  // 0x20000  1<<17  → +49/+4
constexpr int IDX_ARMOR    = 4;  // 0x40000  1<<18  → +50/+5
constexpr int IDX_LEGGIN   = 5;  // 0x80000  1<<19  → +51/+6
constexpr int IDX_GLOVE    = 6;  // 0x100000 1<<20  → +52/+7
constexpr int IDX_SHOES    = 7;  // 0x200000 1<<21  → +53/+8
constexpr int IDX_WEAPON_BASE = 8; // weaponType 1..13 → 8..20

// equipAtb (m_dwEquipAtb) → 一般部位 bucket 索引 (0..7)；非該 10 種值回 -1。
// GT 結構為三層 if/switch 的二分樹（211129..211177 / 211262..211309 / 211454..211516）。
int EquipAtbToBucketIdx(std::uint32_t v9) {
    if (v9 > 0x20000u) {
        if (v9 > 0x100000u) {
            if (v9 == 0x200000u) return IDX_SHOES;
        } else {
            switch (v9) {
                case 0x100000u: return IDX_GLOVE;
                case 0x40000u:  return IDX_ARMOR;
                case 0x80000u:  return IDX_LEGGIN;
            }
        }
    } else if (v9 == 0x20000u) {
        return IDX_SHIELD;
    } else if (v9 > 0x4000u) {
        // 0x8000 (one-hand weapon) / 0x10000 (two-hand weapon) — 武器
        // 由 caller 自行分流到 WeaponItemCnt / WeaponItemInit。
        return -1;
    } else {
        switch (v9) {
            case 0x4000u: return IDX_NECKLACE;
            case 0x1000u: return IDX_CAP;
            case 0x2000u: return IDX_RING;
        }
    }
    return -1;
}

inline bool IsWeaponAtb(std::uint32_t v9) {
    return v9 == 0x8000u || v9 == 0x10000u;
}

// === 對應 GT 211121..211187 / 211246..211261 的職業相符檢查 =================
// 韓文: 자기 직업(또는 그 상위 직업)이 아이템의 장착 가능 직업 비트와
//       하나라도 겹치면 true. 아이템에 직업 제한이 없을 때(qword==0)도 true.
// 中文: 角色當前職業（或其父職業鏈）只要與物品的可裝備職業 bitmask
//       有任一 bit 重疊就回 true；物品 qword==0 表示無職業限制。
// 注意: 父職業鏈用 strClassKindInfo::wTransferableClasses (offset +20) 走訪。
bool ClassCanEquip(stItemKindInfo* item, strClassKindInfo* pClass) {
    const std::uint64_t itemAtb = GetItemClassAtb(item);
    if (itemAtb == 0) {
        // 無職業限制 — GT 直接 goto LABEL_10，不再檢查 pClass。
        return true;
    }
    if (!pClass) return false;
    // 直接職業比對
    if (itemAtb & pClass->qwClassAtb) return true;
    // 沿父職業鏈往上找
    strClassKindInfo* parent = pClass;
    while (true) {
        parent = g_clClassKindInfo.GetClassKindInfo(parent->wTransferableClasses);
        if (!parent) return false;
        if (itemAtb & parent->qwClassAtb) return true;
    }
}

} // namespace

// ============================================================================
// (004FF8B0)  ctor — 把 22 個 bucket 指標、21 個計數歸零
// ============================================================================
cltMoFC_EquipmentInfo::cltMoFC_EquipmentInfo() {
    for (int i = 0; i < 22; ++i) m_ppLists[i] = nullptr;
    for (int i = 0; i < 21; ++i) m_wCounts[i] = 0;
}

// ============================================================================
// (004FF8E0)  dtor — 透過 Free() 把所有 bucket 釋放
// ============================================================================
cltMoFC_EquipmentInfo::~cltMoFC_EquipmentInfo() {
    Free();
}

// ============================================================================
// (004FF8F0)  Free  — GT 211030..211062
//   兩段固定長度的 do-while：(this+4) 8 格 + (this+36) 14 格 = 22 個 ptr。
//   每格非空就 operator delete + 設 nullptr。
// ============================================================================
void cltMoFC_EquipmentInfo::Free() {
    // GT 第一段：offset +4 起 8 個 ptr (m_ppLists[0..7] — 一般裝備 bucket)
    for (int i = 0; i < 8; ++i) {
        if (m_ppLists[i]) {
            ::operator delete(m_ppLists[i]);
            m_ppLists[i] = nullptr;
        }
    }
    // GT 第二段：offset +36 起 14 個 ptr (m_ppLists[8..21] — 武器 13 + 1 預留)
    for (int i = 8; i < 22; ++i) {
        if (m_ppLists[i]) {
            ::operator delete(m_ppLists[i]);
            m_ppLists[i] = nullptr;
        }
    }
}

// ============================================================================
// (004FF940)  Init  — GT 211066..211338
//   兩遍掃描 + qsort 排序：
//     Pass1: 統計每個 bucket 大小（過濾 kind!=HUNT、附魔、職業不符）
//     allocate 21 個 bucket
//     Pass2: 把符合條件 (Pass1 + m_byRareType==0) 的 stItemKindInfo* 寫入 bucket
//     Pass3: 依 m_byLevel qsort 每個 bucket
// ============================================================================
void cltMoFC_EquipmentInfo::Init(std::uint16_t classKind) {
    // 1. 釋放舊配置 (GT 211089)
    Free();

    // 2. 21 個 bucket count 清零 (GT 211090..211110)
    for (int i = 0; i < 21; ++i) m_wCounts[i] = 0;

    // 3. 取得目前角色的職業資料 (GT 211111)
    strClassKindInfo* pClass = g_clClassKindInfo.GetClassKindInfo(classKind);

    // ===== Pass 1: 計算 bucket 大小 (GT 211112..211190) =====================
    for (std::uint32_t i = 0; i < 0xFFFFu; ++i) {
        const std::uint16_t kindId = static_cast<std::uint16_t>(i);
        stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(kindId);
        if (!item) continue;                              // GT 211117 NULL check
        if (item->m_byItemClass != ITEM_CLASS_HUNT) continue;       // +34 != 1
        if (item->Equip.Hunt.m_byEnchantLevel != 0) continue;       // +214 != 0
        if (!ClassCanEquip(item, pClass)) continue;                 // GT 211121..211187

        const std::uint32_t v9 = item->Equip.m_dwEquipAtb;          // +80
        const int idx = EquipAtbToBucketIdx(v9);
        if (idx >= 0) {
            ++m_wCounts[idx];
        } else if (IsWeaponAtb(v9)) {
            WeaponItemCnt(item);
        }
        // 其他 v9 不識別 → 略過（GT 默契：goto LABEL_32）
    }

    // ===== 配置 21 個 bucket (GT 211191..211222) =============================
    // GT 用 operator new(4 * count)；當 count==0 時 GT 仍配置一個 0-byte block。
    // 我們用 operator new(size_t) 並在 size==0 時改配置 sizeof(void*) 以避免
    // 某些 STL 實作對 0-byte alloc 的差異。語意上 bucket 計數仍是 0，後續不
    // 會去寫入該 bucket，因此額外配的位元組永遠不會被讀寫。
    for (int i = 0; i < 21; ++i) {
        std::size_t bytes = sizeof(stItemKindInfo*) * static_cast<std::size_t>(m_wCounts[i]);
        if (bytes == 0) bytes = sizeof(stItemKindInfo*);
        m_ppLists[i] = static_cast<stItemKindInfo**>(::operator new(bytes));
    }
    // 第 22 槽 m_ppLists[21] 不 alloc — GT 也沒有 *((_DWORD *)this + 22) = new。

    // 重設 21 個 counts 供 Pass2 寫入時當作偏移 (GT 211223..211236)
    for (int i = 0; i < 21; ++i) m_wCounts[i] = 0;

    // ===== Pass 2: 填入 bucket (GT 211237..211316) ==========================
    for (std::uint32_t i = 0; i < 0xFFFFu; ++i) {
        const std::uint16_t kindId = static_cast<std::uint16_t>(i);
        stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(kindId);
        if (!item) continue;
        if (item->m_byItemClass != ITEM_CLASS_HUNT) continue;       // +34 != 1
        if (item->Equip.Hunt.m_byEnchantLevel != 0) continue;       // +214 != 0
        if (item->Equip.Hunt.m_byRareType    != 0) continue;        // +218 != 0  (Pass2 多的條件)
        if (!ClassCanEquip(item, pClass)) continue;

        const std::uint32_t v9 = item->Equip.m_dwEquipAtb;
        const int idx = EquipAtbToBucketIdx(v9);
        if (idx >= 0) {
            m_ppLists[idx][m_wCounts[idx]] = item;
            ++m_wCounts[idx];
        } else if (IsWeaponAtb(v9)) {
            WeaponItemInit(item);
        }
    }

    // ===== Pass 3: 對每個 bucket 依 m_byLevel 升冪排序 (GT 211317..211337) ===
    for (int i = 0; i < 21; ++i) {
        std::qsort(m_ppLists[i],
                   m_wCounts[i],
                   sizeof(stItemKindInfo*),
                   compareItemLevel);
    }
}

// ============================================================================
// (00500030)  WeaponItemCnt — GT 211341..211387
//   依 stItemKindInfo::Equip.Hunt.m_wWeaponType (offset +84) 1..13 累加對應的
//   bucket count；其餘值忽略。
// ============================================================================
void cltMoFC_EquipmentInfo::WeaponItemCnt(stItemKindInfo* a2) {
    const std::uint16_t wt = a2->Equip.Hunt.m_wWeaponType;
    if (wt < 1 || wt > 13) return;
    ++m_wCounts[IDX_WEAPON_BASE + (wt - 1)];
}

// ============================================================================
// (005000E0)  WeaponItemInit — GT 211390..211436
//   依武器類型把指標寫入對應 bucket 的尾端 + 自增 count。
// ============================================================================
void cltMoFC_EquipmentInfo::WeaponItemInit(stItemKindInfo* a2) {
    const std::uint16_t wt = a2->Equip.Hunt.m_wWeaponType;
    if (wt < 1 || wt > 13) return;
    const int idx = IDX_WEAPON_BASE + (wt - 1);
    m_ppLists[idx][m_wCounts[idx]] = a2;
    ++m_wCounts[idx];
}

// ============================================================================
// (00500250)  GetEquipItemListPtr — GT 211439..211591
//
// 入: itemKind = 要查詢的物品 ID (a2)
//     playerLevel = 玩家當前等級 (GT 把 byte 級數當 stItemKindInfo** 傳進來)
// 出: outList = bucket 的指標頭
//     outItemIdx = 該物品在 bucket 的位置（找不到則為 bucket size）
//     outLevelThresholdIdx = 第一個 m_byLevel > playerLevel 的位置
// 回傳: bucket 的 entry 數；對應不到 bucket 時回 0。
// ============================================================================
std::uint16_t cltMoFC_EquipmentInfo::GetEquipItemListPtr(
    std::uint16_t itemKind,
    std::uint8_t  playerLevel,
    stItemKindInfo*** outList,
    std::uint16_t*   outItemIdx,
    std::uint16_t*   outLevelThresholdIdx)
{
    // GT 211449..211452 — 找物品；非 HUNT 直接回 0。
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(itemKind);
    if (!item) return 0;
    if (item->m_byItemClass != ITEM_CLASS_HUNT) return 0;

    const std::uint32_t v9 = item->Equip.m_dwEquipAtb;          // +80
    stItemKindInfo** v10 = nullptr;
    std::uint16_t result = 0;

    // GT 211454..211516 — 一般部位 bucket
    int idx = EquipAtbToBucketIdx(v9);
    if (idx >= 0) {
        v10 = m_ppLists[idx];
        result = m_wCounts[idx];
    } else if (IsWeaponAtb(v9)) {
        // GT 211517..211572 — 武器 bucket：依 m_wWeaponType
        const std::uint16_t wt = item->Equip.Hunt.m_wWeaponType;  // +84
        if (wt >= 1 && wt <= 13) {
            const int widx = IDX_WEAPON_BASE + (wt - 1);
            v10 = m_ppLists[widx];
            result = m_wCounts[widx];
        } else {
            // 不識別的 weapon type — GT default fallthrough 等同 v10=0/result=0
            return 0;
        }
    } else {
        // GT LABEL_38: 不識別的 EquipAtb — 與 IDA 反編譯版相同回傳 0。
        return 0;
    }

    if (!v10) return 0;                                          // GT 211575

    // GT 211577..211581 — 找出 itemKind 在 bucket 內的位置
    std::uint16_t i;
    for (i = 0; i < result; ++i) {
        if (item->m_wKind == v10[i]->m_wKind) break;
    }
    *outItemIdx = i;

    // GT 211583..211588 — 找出第一個 m_byLevel > playerLevel 的位置
    std::uint16_t j;
    for (j = 0; j < result; ++j) {
        if (reinterpret_cast<const std::uint8_t*>(v10[j])[120] > playerLevel) break;
    }
    *outLevelThresholdIdx = j;

    *outList = v10;
    return result;
}
