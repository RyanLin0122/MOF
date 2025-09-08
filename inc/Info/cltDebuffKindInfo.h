#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>

// 由其他模組提供的文字檔管理器（外部全域）
// 與你所提供之 cltTextFileManager 保持一致接口
class cltTextFileManager;
extern cltTextFileManager g_clTextFileManager;

// 與反編譯結果等價的資料結構 (總長 72 bytes)
#pragma pack(push, 1)
struct strDebuffKindInfo
{
    // 0x00
    uint16_t kind;     // 由 TranslateKindCode 轉出的 16-bit 代碼
    uint16_t pad0;     // 與反編譯碼一致：kind 後直接在 +4 寫入第一個 int

    // 0x04 ~ 0x17
    int32_t duration;          // 持續時間
    int32_t duration_boss;     // 持續時間(Boss)
    int32_t period;            // 重複週期
    int32_t period_boss;       // 重複週期(Boss)

    // 0x14
    int32_t damage_type;       // 0: NONE, 1: CONSTANT, 2: RATE_CHP, 3: CALC

    // 0x18 ~ 0x27
    int32_t dmg_param1;
    int32_t dmg_param2;
    int32_t dmg_param3;
    int32_t dmg_param4;

    // 0x28 ~ 0x3F：反編譯碼未使用的區段，保留以對齊位移
    char     reserved[24];

    // 0x40 ~ 0x47
    uint16_t mon_top1;
    uint16_t mon_top2;
    uint16_t mon_bot1;
    uint16_t mon_bot2;
};
#pragma pack(pop)

static_assert(sizeof(strDebuffKindInfo) == 72, "strDebuffKindInfo must be 72 bytes");

class cltDebuffKindInfo
{
public:
    cltDebuffKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltDebuffKindInfo() { Free(); }

    // 依據反編譯碼，完整解析檔案；成功完整讀畢回傳 1，否則 0
    int Initialize(char* filename);

    // 釋放配置的資料
    void Free();

    // 依 kind 代碼查詢
    strDebuffKindInfo* GetDebuffKindInfo(uint16_t kind);

    // 與反編譯碼等價的 KindCode 轉換
    static uint16_t TranslateKindCode(char* s);

    // 供除錯或外部檢視
    inline int Count() const { return m_count; }
    inline const strDebuffKindInfo* Data() const { return m_list; }

private:
    strDebuffKindInfo* m_list; // 連續陣列
    int                m_count;
};
