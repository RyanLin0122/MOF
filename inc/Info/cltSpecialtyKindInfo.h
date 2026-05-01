#pragma once

#include <cstdint>

// Forward declarations
class cltSkillKindInfo;

// ──────────────────────────────────────────────────────────────────────────
// strSpecialtyKindInfo  —  特性記錄（156 bytes，與 mofclient.c 一致）
//
// 來源：mofclient.c 內 cltSpecialtyKindInfo::Initialize 解析 "specialty.txt"
//   每行 42 欄，以 '\t' / '\n' 分隔；index 以 0 起算：
//     [ 0] 전공 아이디    (S####, 5碼)            → wKind                 偏移 +0
//     [ 1] 전공명          (中文：特性名稱)        → 略過                 ──
//     [ 2] 전공명 코드    (中文：名稱DCT碼)       → wTextCode             偏移 +2
//     [ 3] 전공 설명 코드 (中文：說明DCT碼)       → wDescCode             偏移 +4
//     [ 4] 전공 타입      (GENERIC/MAKING/TRANSFORM) → byType            偏移 +6
//     [ 5] 이전 전공 아이디 (中文：先修特性)      → wRequiredSpecialtyKind  偏移 +8
//     [ 6] 요구 전공 포인트 (中文：所需點數)      → byRequiredSpecialtyPt 偏移 +10
//     [ 7] 학년 제한      (中文：學年限制)        → byRequiredGrade       偏移 +11
//     [ 8] 레벨 제한      (中文：等級限制)        → dwRequiredLevel        偏移 +12
//     [ 9] 리소스 아이디  (中文：資源ID, 16進)    → dwResourceID            偏移 +16
//     [10] 도움 아이디    (中文：道具/說明ID)     → wHelpID                 偏移 +20
//     [11] 제작 분류      (中文：製作分類字串)    → 暫存於 String1，僅 MAKING 類型解析寫入 dwMakingCategory[]
//     [12..36] 제작 품목 1~25 (中文：製作品項)  → wMakingItemKinds[25]     偏移 +32
//     [37..41] 추가 스킬 1~5  (中文：附加技能)  → wAcquiredSkillKinds[5]   偏移 +22
// ──────────────────────────────────────────────────────────────────────────
#pragma pack(push, 1)
struct strSpecialtyKindInfo {
    // 韓文: 전공 아이디         | 中文: 特性ID（'S'+4位數，cltSpecialtyKindInfo::TranslateKindCode 編碼後）
    // Offset: +0    | Size: 2
    uint16_t wKind;

    // 韓文: 전공명 코드          | 中文: 特性名稱DCT文本碼（atoi 十進位）
    // Offset: +2    | Size: 2
    uint16_t wTextCode;

    // 韓文: 전공 설명 코드        | 中文: 特性說明DCT文本碼（atoi 十進位）
    // Offset: +4    | Size: 2
    uint16_t wDescCode;

    // 韓文: 전공 타입            | 中文: 特性類型 (1=GENERIC一般, 2=MAKING製作, 3=TRANSFORM變身)
    // Offset: +6    | Size: 1
    uint8_t  byType;

    // 對齊填充 / Alignment padding
    // Offset: +7    | Size: 1
    uint8_t  byPad7;

    // 韓文: 이전 전공 아이디     | 中文: 前置（先修）特性ID（TranslateKindCode 編碼）
    // Offset: +8    | Size: 2
    uint16_t wRequiredSpecialtyKind;

    // 韓文: 요구 전공 포인트     | 中文: 所需特性點數（atoi）
    // Offset: +10   | Size: 1
    uint8_t  byRequiredSpecialtyPt;

    // 韓文: 학년 제한            | 中文: 學年（年級）限制（atoi）
    // Offset: +11   | Size: 1
    uint8_t  byRequiredGrade;

    // 韓文: 레벨 제한            | 中文: 等級限制（atoi 寫入 4 bytes）
    // Offset: +12   | Size: 4
    uint32_t dwRequiredLevel;

    // 韓文: 리소스 아이디        | 中文: 資源ID（sscanf "%x" 16進位）
    // Offset: +16   | Size: 4
    uint32_t dwResourceID;

    // 韓文: 도움 아이디          | 中文: 道具/說明ID（atoi 十進位）
    // Offset: +20   | Size: 2
    uint16_t wHelpID;

    // 韓文: 추가 스킬 1~5        | 中文: 附加技能 1~5（cltSkillKindInfo::TranslateKindCode）
    // Offset: +22   | Size: 10  (5 × 2)
    uint16_t wAcquiredSkillKinds[5];

    // 韓文: 제작 품목 1~25       | 中文: 製作品項 1~25（cltMakingItemKindInfo::TranslateKindCode）
    // Offset: +32   | Size: 50  (25 × 2)
    uint16_t wMakingItemKinds[25];

    // 對齊填充 / Alignment padding
    // Offset: +82   | Size: 2
    uint16_t wPad82;

    // 韓文: 제작 품목 분류       | 中文: 製作品項分類陣列（僅當 byType==MAKING(2) 時填入；
    //                                  解析欄位 [11] 之 "POTION|SCROLL|..." 字串並逐一呼叫
    //                                  cltItemKindInfo::GetMakingCategory(token)；至多 18 筆）
    // Offset: +84   | Size: 72  (18 × 4)
    uint32_t dwMakingCategory[18];
};
#pragma pack(pop)

static_assert(sizeof(strSpecialtyKindInfo) == 156,
              "strSpecialtyKindInfo must be exactly 156 bytes (matches mofclient.c)");

// 特性類型常數（值與 cltSpecialtyKindInfo::GetSpecialtyType 一致）
enum ESpecialtyType : uint8_t {
    SPECIALTY_TYPE_GENERIC   = 1,  // 一般
    SPECIALTY_TYPE_MAKING    = 2,  // 製作
    SPECIALTY_TYPE_TRANSFORM = 3,  // 變身
};

class cltSpecialtyKindInfo {
public:
    cltSpecialtyKindInfo();
    ~cltSpecialtyKindInfo();

    // 與 mofclient.c 同步的全部成員函式
    static void                  InitializeStaticVariable(cltSkillKindInfo* a1);

    int                          Initialize(char* String2);
    void                         Free();

    strSpecialtyKindInfo*        GetSpecialtyKindInfo(uint16_t kind);

    int                          GetSpecialtyList(char a2, uint16_t* a3);
    int                          GetGenericSpecialtyList(uint16_t* a2);
    int                          GetMakingSpecialtyList(uint16_t* a2);
    int                          GetTransformSpecialtyList(uint16_t* a2);

    int                          IsLastLevelSpecialty(uint16_t kind);

    // 反編譯回傳 strSpecialtyKindInfo*（實為 0 / 1）；本實作回傳 int 維持等價
    int                          IsGenericSpeciatly(uint16_t kind);
    int                          IsMakingItemSpecialty(uint16_t kind);
    int                          IsTransformSpecialty(uint16_t kind);

    static uint8_t               GetSpecialtyType(char* s);
    static uint16_t              TranslateKindCode(char* s);

    void                         SetMakingItemCategory(char* String1, uint32_t* a3);

    int                          IsSpecialtySkillKind(uint16_t skillKind);
    uint16_t                     GetSpecialtyKindBySkillKind(uint16_t skillKind);

    int                          IsPassiveSpeciaty(uint16_t kind);
    int                          IsActiveSpeciaty(uint16_t kind);
    int                          IsCircleSpecialty(uint16_t kind);

    int                          CanAddToQuickSlot(uint16_t kind);

    // 反編譯回傳 int（v3 += byte），語意上是 "從 kind 沿先修鏈累計的 byRequiredSpecialtyPt"
    int                          GetSpecialtyTotalPoint(uint16_t kind);

    // 靜態：技能資訊的查詢來源（用於 IsPassive/IsActive/IsCircle）
    static cltSkillKindInfo*     m_pclSkillKindInfo;

    // 內部資料訪問（給 Python 模擬器與單元測試對照用，不影響原始語意）
    int                          GetEntryCount() const { return m_count; }
    const strSpecialtyKindInfo*  GetEntryAt(int idx) const {
        return (idx < 0 || idx >= m_count) ? nullptr : &m_table[idx];
    }

private:
    // 與反編譯完全一致的 layout：
    //   +0x00  m_table  strSpecialtyKindInfo*  資料陣列（operator new(156 * count)）
    //   +0x04  m_count  int                    記錄數
    strSpecialtyKindInfo* m_table;
    int                   m_count;
};
