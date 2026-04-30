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

// 호칭 획득 조건 (稱號取得條件) — TranslateGetCondition 反編譯結果
//   1: COMPLETEQUEST  – 퀘스트 완수      (完成任務)
//   2: GETSKILL       – 스킬 습득        (學會技能)
//   3: SETEMBLEM      – 엠블렘 장착      (裝備徽章)
//   4: GETITEM        – 아이템 획득      (取得道具)
//   5: NONE           – 조건 없음        (無條件)
//   6: KILLMONSTER    – 몬스터 처치      (擊殺怪物)
//   *기타*            – 0 (反編譯算式 (-(stricmp!=0)) & 0xFB + 5 在 32-bit 下回傳 0)
enum TitleGetCondition : int32_t {
    TGC_NONE_UNKNOWN  = 0,  // 未匹配 / 알 수 없음
    TGC_COMPLETEQUEST = 1,
    TGC_GETSKILL      = 2,
    TGC_SETEMBLEM     = 3,
    TGC_GETITEM       = 4,
    TGC_NONE          = 5,
    TGC_KILLMONSTER   = 6
};

// strTitleKindInfo — 28 bytes, packed (mofclient.c v7+=28)
//
//   offset  size  field         type   韓文 / 中文 註解
//   ------  ----  ------------  ----   ----------------------------------------
//    +0      2    title_code    u16    호칭_ID (稱號代碼) – Txxxx 經 ((toupper+31)<<11)|atoi 編碼後的 16-bit
//    +2      2    name_code     u16    이름_code (名稱資源代碼) – e.g. 54001
//    +4      4    lv_min        i32    최저 레벨 (最低等級需求), 0 表示不限
//    +8      4    lv_max        i32    최고 레벨 (最高等級需求), 0 表示不限
//   +12      4    condition     i32    획득 조건 (取得條件) — TitleGetCondition
//   +16      2    param1_code   u16    획득 조건 매개변수 1 (條件參數 1)
//                                          COMPLETEQUEST → 任務代碼 (Qxxxx)
//                                          GETSKILL      → 技能代碼 (Axxxxx)
//                                          SETEMBLEM     → 徽章代碼 (Exxxx)
//                                          GETITEM       → 道具代碼 (Fxxxx)
//                                          KILLMONSTER   → 怪物代碼 (Jxxxx)
//                                          NONE          → 不存值 (memset 0)
//   +18      2    _pad          u16    對齊保留 (反編譯 memset 0 後從未存取)
//   +20      4    param2        i32    획득 조건 매개변수 2 (條件參數 2)
//                                          GETITEM     → 數量
//                                          KILLMONSTER → 擊殺數量
//                                          NONE        → 不存值 (memset 0)
//   +24      4    param3        i32    획득 조건 매개변수 3 (條件參數 3)
//                                          KILLMONSTER → 額外條件值 (param1==0 時必須非 0)
//                                          其他        → 0
#pragma pack(push, 1)
struct strTitleKindInfo {
    uint16_t title_code;   // +0   호칭_ID            稱號代碼
    uint16_t name_code;    // +2   이름_code          名稱資源代碼
    int32_t  lv_min;       // +4   최저 레벨          最低等級
    int32_t  lv_max;       // +8   최고 레벨          最高等級
    int32_t  condition;    // +12  획득 조건          取得條件 (TitleGetCondition)
    uint16_t param1_code;  // +16  매개변수 1         條件參數 1 (16-bit kind code)
    uint16_t _pad;         // +18  padding            對齊保留
    int32_t  param2;       // +20  매개변수 2         條件參數 2
    int32_t  param3;       // +24  매개변수 3         條件參數 3
};
#pragma pack(pop)
static_assert(sizeof(strTitleKindInfo) == 28, "strTitleKindInfo must be 28 bytes");

// cltTitleKindInfo — 호칭 종류 정보 (稱號種類資訊)
//
//   This-pointer layout (mofclient.c reads via *(_DWORD*)this 與 *((_DWORD*)this+1))
//      +0  void*         m_list   호칭 데이터 배열   稱號資料陣列 (operator new[m_count*28])
//      +4  int32_t       m_count  호칭 개수          稱號筆數 (counted by 1st-pass fgets loop)
class cltTitleKindInfo {
public:
    cltTitleKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltTitleKindInfo() { Free(); }

    // mofclient.c:338263 — 解析 titlekindinfo.txt
    // 回傳值與反編譯 v27 一致：
    //   1 : 開檔成功且
    //         (a) 資料區為空 (LABEL_35) 或
    //         (b) 完整跑到 EOF (最後一次 fgets 回 NULL → LABEL_35)
    //   0 : fopen 失敗、前 3 行 header 不足、或解析中途任一欄位/條件失敗
    int Initialize(char* filename);

    void Free();

    // mofclient.c:338452 — 5-char Txxxx 編碼 → 16-bit
    static uint16_t TranslateKindCode(const char* s);

    strTitleKindInfo* GetTitleKindInfo() { return m_list; }
    int               GetTitleKindNum() const { return m_count; }

    // 線性查找 title_code；找不到回 nullptr
    strTitleKindInfo* GetTitleKindInfo(uint16_t code);

    // mofclient.c:338503 — 條件字串轉 TitleGetCondition
    // 對齊反編譯：未知字串回傳 0（由 32-bit 整數溢位產生）
    static int TranslateGetCondition(const char* s);

private:
    strTitleKindInfo* m_list;   // +0  稱號表 (m_count 筆)
    int               m_count;  // +4  稱號筆數
};
