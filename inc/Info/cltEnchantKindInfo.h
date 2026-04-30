#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "Info/cltItemKindInfo.h"
#include "global.h"

// =============================================================================
// strEnchantKindInfo
// 反編譯位置：mofclient.c:300026 起 cltEnchantKindInfo::Initialize
//             以及 GetEnchantKindInfo 系列在 +0/+2/+4 的 WORD 比對、
//             以及 v7+18(_WORD*) 等指標位移。
// 單筆強化規則記錄；操作記憶體佈局與大小（84 bytes）皆對齊反編譯碼。
//
// 韓文／中文欄位對照（行頭來自 enchant.txt 第 3 行欄位列）：
// =============================================================================
#pragma pack(push, 1)
struct strEnchantKindInfo
{
    // 0x00  인챈트 ID                    強化規則 ID（A0001… 經 TranslateKindCode 轉碼）
    uint16_t EnchantID;
    // 0x02  인챈트 결과 아이템           強化後的結果物品（N**** 物品代碼）
    uint16_t ResultItem;
    // 0x04  필요 무기 아이템             被強化的需求武器物品（H**** / R**** 物品代碼）
    uint16_t RequiredWeaponItem;

    // 0x06..0x0F (5 * u16)
    //   [0] 필요 원석 아이템             需求原石（基本石頭，G****）
    //   [1] 빛나는 원석 I                閃耀原石 I
    //   [2] 빛나는 원석 II               閃耀原石 II
    //   [3] 빛나는 원석 III              閃耀原石 III
    //   [4] 이벤트원석                   活動原石
    uint16_t StoneKind[5];

    // 0x10..0x23 (5 * i32)  確率(천분률)               成功率（千分比 0..1000，必須 != 0）
    int32_t  SuccessPermille[5];
    // 0x24..0x37 (5 * i32)  증발 확률(천분율)         蒸發率（千分比，必須 < 1000；可為 0）
    int32_t  EvaporatePermille[5];
    // 0x38..0x4B (5 * i32)  손상 확률(천분율)         損壞率（千分比，必須 < 1000；可為 0）
    int32_t  DamagePermille[5];

    // 0x4C  비용                         強化費用（金錢，必須 > 0）
    int32_t  Cost;
    // 0x50  옵션(텍스트코드)             選項說明文字代碼（DCTTextManager 的字串編號）
    uint16_t OptionTextCode;

    // 0x52..0x53  padding（與反編譯碼 84 bytes 對齊一致；該 2 bytes 不被讀寫）
    uint16_t ReservedPadding;
};
static_assert(sizeof(strEnchantKindInfo) == 84, "strEnchantKindInfo size must be 84 bytes");
#pragma pack(pop)

// =============================================================================
// cltEnchantKindInfo
// 反編譯位置：mofclient.c:8528-8538 宣告、300026-300340 實作
// 物件記憶體佈局（與 IDA 之 *(_DWORD*)this 與 *((_DWORD*)this+1) 對齊）：
//   +0   strEnchantKindInfo* table   ：整塊 84 * count bytes 的資料表起點
//   +4   int                count    ：表格筆數（資料行數）
// =============================================================================
class cltEnchantKindInfo
{
public:
    cltEnchantKindInfo() : table_(nullptr), count_(0) {}
    ~cltEnchantKindInfo() { Free(); }

    // 讀入 enchant.txt（透過 cltTextFileManager 開檔）。
    // 回傳 1=成功（含「沒有資料行」的情境），0=任一筆解析失敗或開檔失敗。
    // 注意：解析失敗時 *不* 會釋放已配置的 table；交給解構式 Free() 處理。
    int Initialize(char* filename);

    void Free();

    // 依 EnchantID 線性搜尋（比對 +0 的 WORD）。
    strEnchantKindInfo* GetEnchantKindInfo(uint16_t id);

    // 依 RequiredWeaponItem（需求武器代碼，比對 +4 的 WORD）線性搜尋。
    strEnchantKindInfo* GetEnchantKindInfoByReqHuntItemKind(uint16_t reqWeaponKind);

    // 是否存在以該武器代碼為 RequiredWeaponItem 的規則。
    int IsEnchantableItem(uint16_t reqWeaponKind);

    // 是否為「強化後的結果物品」（比對 +2 的 WORD）。
    int IsEnchantedItem(uint16_t resultItemKind);

    // 給「強化後物品」反查「原始（被消耗）武器」（回傳該筆 +4 的 WORD）。
    uint16_t GetDamagedItem(uint16_t resultItemKind);

    // 5 字元 KindCode（A0001 / N0005 / G1231…）轉 16-bit Kind：
    //   ((toupper(c) + 31) << 11) | atoi(rest)，rest 必須 < 0x800。
    static uint16_t TranslateKindCode(char* code);

    inline const strEnchantKindInfo* data() const { return table_; }
    inline int count() const { return count_; }

private:
    strEnchantKindInfo* table_;  // +0：資料表起點
    int                 count_;  // +4：筆數

    // 與全域 ::IsDigit (mofclient.c:342909) 行為等價的私有版本。
    static bool IsDigit(const char* t);
};
