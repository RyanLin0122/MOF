#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "global.h"

// 7 個 int = 28 bytes，與反編譯位移完全一致
#pragma pack(push, 1)
struct strMonsterToleranceKindInfo
{
    // 對應反編譯中 v7[0..6] 的寫入順序
    int threshold;              // v7[0]  - 몬스터 잡은수 (擊殺數門檻)
    int attack_up_100;          // v7[1]  - 攻擊上升(100分率)
    int defense_up_100;         // v7[2]  - 防禦上升(100分率)  ※檔案第2欄，但實際寫到 [2]
    int skill_attack_up_100;    // v7[3]  - 技能攻擊上升(100分率)
    int hit_up_1000;            // v7[4]  - 命中上升(1000分率)
    int dodge_up_1000;          // v7[5]  - 迴避上升(1000分率)
    int crit_up_1000;           // v7[6]  - 爆擊機率上升(1000分率)
};
#pragma pack(pop)

static_assert(sizeof(strMonsterToleranceKindInfo) == 28, "strMonsterToleranceKindInfo must be 28 bytes");

class cltMonsterToleranceKindInfo
{
public:
    cltMonsterToleranceKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltMonsterToleranceKindInfo() { Free(); }

    // 成功完整解析回傳 1；否則 0
    int Initialize(char* filename);
    void Free();

    // 查詢（區間/精確）
    strMonsterToleranceKindInfo* GetMonsterToleranceKindInfo(unsigned int killCount);
    strMonsterToleranceKindInfo* GetMonsterToleranceKindInfo_exactly(unsigned int threshold);

    // 便利存取
    inline int Count() const { return m_count; }
    inline const strMonsterToleranceKindInfo* Data() const { return m_list; }

private:
    strMonsterToleranceKindInfo* m_list;
    int                          m_count;
};
