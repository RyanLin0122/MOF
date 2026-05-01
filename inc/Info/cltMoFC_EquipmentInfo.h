#pragma once

// =============================================================================
// cltMoFC_EquipmentInfo
// 還原自 mofclient.c 0x4FF8B0..0x500480（行 211002~211591）。
//
// 韓文: 캐릭터 장착 가능 아이템 분류 / 中文: 角色可裝備物品分類器
//
// 角色登入完成 (cltMyCharData::Initialize) 時呼叫 Init(classKind)，會掃過
// 全部 cltItemKindInfo (最多 65535 筆)，把符合「目前職業 + 衍生父職業可裝備
// + 普通 (非附魔/非稀有) HUNT 類別」的物品依「裝備部位 m_dwEquipAtb / 武器
// 種類 m_wWeaponType」分到 21 個 bucket：
//   bucket  0..7  → 一般部位 (CAP/RING/NECKLACE/SHIELD/ARMOR/LEGGIN/GLOVE/SHOES)
//   bucket  8..20 → 武器類型 1..13 (短劍/長劍/雙手劍/槌/斧/槍/弓/銃/法杖/雙刀/弩/魔法書/聖物)
// 每個 bucket 內依 m_byLevel (offset +120) 升冪 qsort，便於 UI 顯示。
//
// 並未直接解析 .txt — 所有資料來自已 Initialize 完畢的 g_clItemKindInfo
// （itemkindinfo.txt + item_hunt.txt 等共 6 檔案）與 g_clClassKindInfo
// （ClassKindInfo.txt）。
//
// === 原 binary memory layout (x86, 共 136 bytes，cltMyCharData +40576) ===
//   +  0  vfptr                                      (4)
//   +  4  void* m_ppLists[8]   ← 一般裝備 bucket     (32)
//   + 36  void* m_ppLists[14]  ← 武器 bucket(13)+1unused (56)
//   + 92  uint16_t m_wCounts[21] ← 21 個 bucket 計數 (42)
//   +134  padding to 136
//
// 本 C++ 還原以「自然 x64 對齊 + 命名陣列」表現相同語意（不需與 GT bytewise
// 等大）；cltMyCharData 在我們的 port 採用「外掛全域子系統」設計，所以這裡
// 保留 22 槽指標是為了忠實對應 GT Free 迴圈 (8+14)。
// =============================================================================

#include <cstdint>

struct stItemKindInfo;

class cltMoFC_EquipmentInfo {
public:
    // ---------------- ctor / dtor ----------------------------------------
    // (004FF8B0)  cltMoFC_EquipmentInfo()
    cltMoFC_EquipmentInfo();
    // (004FF8E0)  ~cltMoFC_EquipmentInfo() — 呼叫 Free()
    virtual ~cltMoFC_EquipmentInfo();

    // (004FF8F0)  Free  — 把 22 個 bucket 釋放並清為 nullptr。
    void Free();

    // (004FF940)  Init  —
    //   韓文 입력: a2 = 자신의 클래스 종류(class kind)
    //   中文 輸入: classKind = 角色職業 ID (例 'F0001'.. 經 TranslateKindCode)
    //   兩遍掃描 cltItemKindInfo (Pass1 計數、Pass2 填表) 後 qsort 排序。
    void Init(std::uint16_t classKind);

    // (00500030)  WeaponItemCnt  — 依武器類型 (Hunt.m_wWeaponType, 1..13)
    //                              累加對應 bucket 的計數 (Pass1 用)。
    void WeaponItemCnt(stItemKindInfo* info);

    // (005000E0)  WeaponItemInit — 依武器類型把指標寫入對應 bucket 並 ++count。
    void WeaponItemInit(stItemKindInfo* info);

    // (00500250)  GetEquipItemListPtr —
    //   韓文 의미: 같은 부위/같은 무기 종류의 정렬된 아이템 목록을 돌려준다.
    //   中文: 取得「與 itemKind 同部位/同武器類型」已排序的可裝備清單，
    //         同時回填：a4 -> 清單頭指標、a5 -> 該物品在清單內的索引、
    //         a6 -> 第一個等級 > 玩家等級的索引（用於 UI 灰階）。
    //   參數 a3 在 GT 雖型別為 stItemKindInfo**，實際上是 player level
    //   經 LOBYTE() 寫入 pointer slot 後傳進來，於本 port 改為清楚的
    //   uint8_t playerLevel。
    //   回傳: 該 bucket 的元素數；找不到對應 bucket 時回 0。
    std::uint16_t GetEquipItemListPtr(std::uint16_t itemKind,
                                      std::uint8_t  playerLevel,
                                      stItemKindInfo*** outList,
                                      std::uint16_t*   outItemIdx,
                                      std::uint16_t*   outLevelThresholdIdx);

private:
    // 22 個 bucket 指標（GT layout）：
    //   m_ppLists[0..7]   一般部位 — 對應 EquipAtb 0x1000/0x2000/0x4000/
    //                     0x20000/0x40000/0x80000/0x100000/0x200000
    //                     (CAP/RING/NECKLACE/SHIELD/ARMOR/LEGGIN/GLOVE/SHOES)
    //   m_ppLists[8..20]  武器 — m_wWeaponType 1..13
    //   m_ppLists[21]     未使用（GT Free 迴圈會清，但 Init 不會配）
    //
    // GT byte offsets (x86): m_ppLists[0]=+4, m_ppLists[7]=+32,
    //                        m_ppLists[8]=+36, m_ppLists[21]=+88
    stItemKindInfo** m_ppLists[22];

    // 21 個 bucket 計數 (與前 21 個指標一一對應)。
    // GT byte offsets (x86): m_wCounts[0]=+92, m_wCounts[20]=+132
    std::uint16_t m_wCounts[21];
};

extern cltMoFC_EquipmentInfo g_clMoFC_EquipmentInfo;
