#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "Info/cltItemKindInfo.h"
#include "global.h"

#pragma pack(push, 1)
// 單筆強化規則（精準對齊反編譯碼，大小 = 84 bytes）
struct strEnchantKindInfo
{
    // 0x00
    uint16_t EnchantID;            // 인챈트 ID  // 強化ID (A0001… 轉為16位代碼)
    // 0x02
    uint16_t ResultItem;           // 인챈트 결과 아이템 // 強化結果物品 (N****)
    // 0x04
    uint16_t RequiredWeaponItem;   // 필요 무기 아이템   // 需求武器物品 (H****)

    // 0x06 ~ 0x0F
    uint16_t StoneKind[5];         // 필요 원석 아이템 / 빛나는 원석 I/II/III / 이벤트원석
    // 需求原石 / 閃耀原石 I/II/III / 活動原石 (G****)

// 0x10 ~ 0x23
    int32_t  SuccessPermille[5];   // 확률(천분률)     // 成功率(千分比) 0..1000
    // 0x24 ~ 0x37
    int32_t  EvaporatePermille[5]; // 증발 확률(천분율) // 蒸發率(千分比) <1000
    // 0x38 ~ 0x4B
    int32_t  DamagePermille[5];    // 손상 확률(천분율) // 損壞率(千分比) <1000

    // 0x4C
    int32_t  Cost;                 // 비용             // 費用 (>0)
    // 0x50
    uint16_t OptionTextCode;       // 옵션(텍스트코드) // 選項(文字代碼)
    // 0x52
    uint16_t ReservedPadding = 0;  // 對齊補位，確保單筆 84 bytes
};
static_assert(sizeof(strEnchantKindInfo) == 84, "strEnchantKindInfo size must be 84 bytes");
#pragma pack(pop)

class cltEnchantKindInfo
{
public:
    cltEnchantKindInfo() : table_(nullptr), count_(0) {}
    ~cltEnchantKindInfo() { Free(); }

    // 讀入 enchant.txt（以 cltTextFileManager 解封包與開檔）
    // 回傳 1=成功，0=失敗（完全對齊反編譯邏輯）
    int Initialize(char* filename);

    void Free();

    // 依 EnchantID（A0001… 轉碼後）取得規則指標
    strEnchantKindInfo* GetEnchantKindInfo(uint16_t id);

    // 依 RequiredWeaponItem（需求武器代碼）取得規則指標
    strEnchantKindInfo* GetEnchantKindInfoByReqHuntItemKind(uint16_t reqWeaponKind);

    // 是否存在可被強化的「需求武器代碼」
    int IsEnchantableItem(uint16_t reqWeaponKind);

    // 是否為強化後的結果物品（ResultItem）
    int IsEnchantedItem(uint16_t resultItemKind);

    // 由「強化後物品」反查「被損壞/原始武器」（對齊反編譯：回傳 RequiredWeaponItem）
    uint16_t GetDamagedItem(uint16_t resultItemKind);

    // A0001/N0005/G1231… 字串轉 16位 Kind Code（完整還原反編譯版位元壓縮）
    static uint16_t TranslateKindCode(char* code);

    // 讀取後的資料表與筆數（如需除錯/驗證可取用）
    inline const strEnchantKindInfo* data() const { return table_; }
    inline int count() const { return count_; }

private:
    strEnchantKindInfo* table_;
    int                 count_;
    bool IsDigitStr(const char* t);
};