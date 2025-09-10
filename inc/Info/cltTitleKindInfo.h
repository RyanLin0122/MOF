#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include "global.h"
#include "Info/cltQuestKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Info/cltEmblemKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltCharKindInfo.h"

// 取得條件枚舉（與反編譯 switch 值一一對應）
enum TitleGetCondition : int32_t {
    TGC_COMPLETEQUEST = 1,  // COMPLETEQUEST
    TGC_GETSKILL = 2,  // GETSKILL
    TGC_SETEMBLEM = 3,  // SETEMBLEM
    TGC_GETITEM = 4,  // GETITEM
    TGC_NONE = 5,  // NONE
    TGC_KILLMONSTER = 6   // KILLMONSTER
};

// 與反編譯 28 bytes 版型一致
#pragma pack(push, 1)
struct strTitleKindInfo {
    // 호칭_ID (稱號ID)：Txxxx 的壓縮代碼（以 TranslateKindCode 轉出）
    uint16_t title_code;

    // 이름_code (名稱code)：文字資源/名稱代碼（檔案第3欄，例：54001）
    uint16_t name_code;

    // lv_min (最低等級)：最小等級需求（0 表示不限制）
    int32_t  lv_min;

    // lv_max (最高等級)：最大等級需求（0 表示不限制）
    int32_t  lv_max;

    // condition (取得條件)：TitleGetCondition
    int32_t  condition;

    // param1 (參數1)：依 condition 不同而異：
    //  - COMPLETEQUEST：任務代碼（Qxxxx）
    //  - GETSKILL：技能代碼（Axxxxx）
    //  - SETEMBLEM：徽章代碼（Exxxx）
    //  - GETITEM：道具代碼（Fxxxx）
    //  - KILLMONSTER：怪物代碼（Jxxxx）
    uint16_t param1_code;

    // 對齊 / 保留（反編譯顯示在 +18 位置未直接存取，保持版型一致）
    uint16_t _pad;

    // param2 (參數2)：
    //  - GETITEM：數量
    //  - KILLMONSTER：數量
    //  - 其他：依格式檔通常為 0
    int32_t  param2;

    // param3 (參數3)：
    //  - KILLMONSTER：額外條件/群組代碼（0 表示無）
    //  - 其他：通常為 0
    int32_t  param3;
};
#pragma pack(pop)
static_assert(sizeof(strTitleKindInfo) == 28, "strTitleKindInfo size mismatch");

// cltTitleKindInfo 類別
class cltTitleKindInfo {
public:
    cltTitleKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltTitleKindInfo() { Free(); }

    // 依反編譯行為：成功完整讀畢回傳 1；開檔失敗或中途解析失敗回傳 0
    int Initialize(char* filename);

    void Free();

    // 與反編譯一致的 16-bit 代碼轉換（必須長度==5）
    static uint16_t TranslateKindCode(const char* s);

    // 回傳整表指標 / 數量
    strTitleKindInfo* GetTitleKindInfo() { return m_list; }
    int GetTitleKindNum() const { return m_count; }

    // 依 title_code 查找指定一筆；找不到回傳 nullptr
    strTitleKindInfo* GetTitleKindInfo(uint16_t code);

    // 解析 condition 字串到枚舉值（與反編譯相同分支與 fallback）
    static int TranslateGetCondition(const char* s);

private:
    strTitleKindInfo* m_list;
    int               m_count;
};
