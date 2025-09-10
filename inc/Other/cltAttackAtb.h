#pragma once
#include <cstdint>
#include <cstddef>

// 攻擊屬性（僅供參考，回傳值以反編譯為主）
enum AttackAtb : int {
    ATB_FIRE = 0,
    ATB_ICE = 1,
    ATB_LIGHTNING = 2,
    ATB_EARTH = 3,
    ATB_HOLY = 4,
    ATB_DARK = 5
};

// 目標屬性（僅供參考，回傳值以反編譯為主）
enum TargetAtb : int {
    TGT_MONSTER_ETC = 0,
    TGT_MONSTER_FIRE = 1,
    TGT_MONSTER_ICE = 2,
    TGT_MONSTER_LIGHTNING = 3,
    TGT_BEAST = 4,
    TGT_UNDEAD = 5,
    TGT_DEMON = 6,
    TGT_DRAGON = 7
};

// 完整還原：此類本體即為一個 6×8×100000 的連續 int 陣列，
// 以 [攻擊屬性][目標屬性][原始傷害] 索引。
// 注意：物件尺寸約 19.2MB，請避免建立在堆疊上。
class cltAttackAtb {
public:
    // 反編譯建構子只回傳 this，無初始化；為安全仍提供明確函式 Initialize()。
    cltAttackAtb();

    // 依反編譯逐項填表（使用位元魔數還原，對應 floor(v/100)）
    void Initialize();

    // 調整後傷害查表：
    // a2: 攻擊屬性(0..5)，a3: 目標屬性(0..7)，a4: 原始傷害
    // - 若 a2/ a3 超界 → 回傳 0
    // - 若 a4 >= 99999 → 以 99999 索引
    // - 若 a4 < 0 → 不修正（與反編譯一致，可能越界）
    int AdjustDamage(int a2, int a3, int a4) const;

    // 大小寫不敏感字串→攻擊屬性編號
    // FIRE→0, ICE→1, LIGHTNING→2, EARTH→3, HOLY→4, DARK→5, 其他→3 (EARTH)
    static int GetAttackAtb(const char* s);

    // 大小寫不敏感字串→目標屬性編號
    // MONSTER_ETC→0, MONSTER_FIRE→1, MONSTER_ICE→2, MONSTER_LIGHTNING→3,
    // BEAST→4, UNDEAD→5, DEMON→6, DRAGON→7, 其他→0 (MONSTER_ETC)
    static int GetAttackTargetAtb(const char* s);

private:
    // 連續記憶體，需符合 this[800000*a2 + 100000*a3 + k] 的索引節距
    // 其中 800000 = 8*100000, 100000 = 每一段的長度
    int m_tbl[6][8][100000];
};
