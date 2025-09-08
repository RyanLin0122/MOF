#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"
#include "Info/cltInstantDungeonMapKindInfo.h"

// 與反編譯佈局一致（總長 44 bytes）
#pragma pack(push, 1)
struct strInstantDungeonKindInfo
{
    // +0
    uint16_t kind;              // TranslateKindCode("S0001"...)

    // +2
    uint16_t base_indun;        // TranslateKindCode(BaseIndun_ID 或 0)

    // +4
    uint16_t name_text_code;    // 인던 이름 텍스트코드 (如 55001)

    // +6
    uint8_t  level_limit;       // 레벨 제한
    uint8_t  _rsv7{ 0 };          // 未使用；僅用於對齊

    // +8 ~ +24
    uint16_t key_item;          // 生成 아이템 ID(열쇠아이템) (Qxxxx)
    uint16_t npc_id;            // NPC ID (Nxxxx)
    uint16_t boss_id;           // 보스몬스터 ID (Jxxxx)
    uint16_t random_spawn;      // 랜덤몬스터등장 (0/4000)
    uint16_t slow_id;           // 느림보몬스터ID (Jxxxx)
    uint16_t slow_prob;         // 느림보몬스터확률
    uint16_t power_id;          // 파워맨몬스터ID (Jxxxx)
    uint16_t power_prob;        // 파워맨몬스터확률
    uint16_t guide_text_code;   // 인던 안내 텍스트 코드

    // +26
    uint16_t _rsv26{ 0 };         // 未使用；使 +28 對齊為 DWORD

    // +28
    uint32_t time_limit;        // timelimit (秒)

    // +32
    uint32_t loading_res_id;    // 로딩리소스ID（十六進位字串以 %x 讀入）

    // +36 ~ +42
    uint16_t map_id[4];         // i_map1_id..i_map4_id（Dxxxx）
};
#pragma pack(pop)

static_assert(sizeof(strInstantDungeonKindInfo) == 44, "strInstantDungeonKindInfo must be 44 bytes");

class cltInstantDungeonKindInfo
{
public:
    cltInstantDungeonKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltInstantDungeonKindInfo() { Free(); }

    // 成功完整讀畢回傳 1；否則 0。檔案「無資料行」視為成功（與反編譯一致）。
    int Initialize(char* filename);
    void Free();

    strInstantDungeonKindInfo* GetInstantDungeonKindInfo(uint16_t kind);
    strInstantDungeonKindInfo* GetInstantDungeonKindInfoByItem(uint16_t item_code);
    uint16_t                   GetInstantDungeonKindByItem(uint16_t item_code);
    strInstantDungeonKindInfo* GetInstantDungeonKindInfoByIndex(int idx); // 注意：反編譯允許 idx==count

    // 與反編譯一致的 5 碼代碼轉換（超過 2047 會跳錯誤視窗）
    static uint16_t TranslateKindCode(char* s);

    inline int Count() const { return m_count; }
    inline const strInstantDungeonKindInfo* Data() const { return m_list; }

private:
    // 工具
    static bool IsAlphaNum(const char* t) {
        if (!t || !*t) return false;
        for (const unsigned char* p = (const unsigned char*)t; *p; ++p)
            if (!std::isalnum(*p)) return false;
        return true;
    }

private:
    strInstantDungeonKindInfo* m_list;
    int                        m_count;
};
