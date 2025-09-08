#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>

#include "global.h"
#include "Info/cltItemKindInfo.h"

// 與反編譯記憶體佈局一致：每筆 72 bytes
// 參照 ClassKindInfo.txt 欄位（\t 分隔）：
//  1  클래스 아이디 (3 碼字母；或 "NONE")
//  2  클래스 이름 (韓文名稱；略過)
//  3  클래스 이름 코드 (數值; WORD)
//  4  전직 가능 클래스 (3 碼字母; WORD)
//  5  전직 단계 (BYTE)
//  6  최소 레벨 (BYTE)
//  7  타겟탐색범위 (DWORD)
//  8~16  各種最低屬性/課程熟練度 (9 個 WORD)
// 17  코멘트 코드 (WORD)
// 18  마크이미지 블록 아이디 (WORD; 常見為小數值索引)
// 19~21  三個 DWORD（依原檔：HP/Mana 與一個額外整數；依資料均為整數）
// 22  전직시 주어지는 아이템1 코드 (5 碼，如 J0001/A0141；或 "0")
// 23  전직시 주어지는 아이템1 수량 (WORD)
// 24  전직시 주어지는 아이템2 코드 (同上；或 "0")
// 25  전직시 주어지는 아이템2 수량 (WORD)
#pragma pack(push, 1)
struct strClassKindInfo
{
    // +0
    uint16_t kind;                // 3 碼類別代碼（TranslateKindCode），"NONE"→0

    // +2..+7（未於本函式族使用；保留）
    uint16_t rsv02 = 0;
    uint32_t rsv04 = 0;

    // +8
    uint64_t atb;                 // 類別屬性位（Initialize 依序配置為 1,2,4,...）

    // +16
    uint16_t name_code;           // 클래스 이름 코드

    // +18
    uint8_t  job_step;            // 전직 단계
    uint8_t  rsv19 = 0;

    // +20
    uint16_t from_class;          // 전직 가능 클래스（3 碼代碼；NONE→0）

    // +22
    uint8_t  min_level;           // 최소 레벨
    uint8_t  rsv23 = 0;

    // +24 ~ +40：最低屬性/熟練度（9 個 WORD）
    uint16_t min_attack;
    uint16_t min_dex;
    uint16_t min_con;
    uint16_t min_int;
    uint16_t mastery_sword;
    uint16_t mastery_magic;
    uint16_t mastery_archery;
    uint16_t mastery_priest;
    uint16_t mastery_thief;

    // +42（未用；對齊）
    uint16_t rsv42 = 0;

    // +44
    uint32_t target_search_range; // 타겟탐색범위

    // +48
    uint16_t comment_code;        // 코멘트 코드
    uint16_t mark_img_block_id;   // 마크이미지 블록 아이디

    // +52 轉職贈品 1
    uint16_t item1_code;          // 物品代碼（cltItemKindInfo::TranslateKindCode；"0"→0）
    uint16_t item1_count;

    // +56 轉職贈品 2
    uint16_t item2_code;          // 同上
    uint16_t item2_count;

    // +60 ~ +68（3 個 DWORD；依原表為 HP/Mana 及額外整數）
    uint32_t base_buff_uses;      // 기본 지급 버프 사용수（常見於資料）
    uint32_t hp_regen;            // HP(맵 자동회복량 상승)
    uint32_t mana_regen;          // Mana(맵 자동회복량 상승)
};
#pragma pack(pop)

static_assert(sizeof(strClassKindInfo) == 72, "strClassKindInfo must be 72 bytes");

class cltClassKindInfo
{
public:
    cltClassKindInfo();
    ~cltClassKindInfo() { Free(); }

    // 成功完整解析回傳 1；否則 0
    int Initialize(char* filename);
    void Free();

    // 3 碼字串（A..Z）→ 15-bit 代碼（每字 5 bits），長度≠3 回傳 0
    static uint16_t TranslateKindCode(char* s);

    // 查詢
    uint16_t GetTotalClassNum() const { return m_count; }
    strClassKindInfo* GetClassKindInfo(uint16_t kind);
    strClassKindInfo* GetClassKindInfoByIndex(unsigned int idx);
    strClassKindInfo* GetClassKindInfoByAtb(uint64_t atb);

    // 以位元組合（mask）找出對應的類別 kind 陣列（逐一 bit 檢索），回傳寫入數量
    // 注意：呼叫端需確保 a3 至少可容納 64 筆
    int GetClassKindsByAtb(uint64_t mask, uint16_t* a3);

    // 由字串 "FIG|MAG|..." 轉出屬性 bitmask 的低 32 位（與反編譯行為一致：只回傳低位）
    int GetClassAtb(char* s);
    uint64_t GetClassAtb(const char* s);

private:
    // 小工具：檢查整段是否為「0..9」數字
    static bool IsDigitStr(const char* t);

private:
    strClassKindInfo* m_list = nullptr; // 連續陣列
    uint16_t          m_count = 0;
};
