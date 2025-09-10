#include "Other/cltAttackAtb.h"
#include <cstring>
#include <cctype>

cltAttackAtb::cltAttackAtb()
{
    // 反編譯版本的建構子僅 `return this;`，不做任何事情。
    // 真正填表放在 Initialize()。
}

void cltAttackAtb::Initialize()
{
    // 完整還原反編譯的三層迴圈與位元魔數：
    // 內層每次 v1 += 110，寫入 ((((1374389535 * v1) >> 32) >> 5) >>31)+...
    // 實際上此為除以 100 的快速整除：floor(v1 / 100)。
    // 這裡以相同魔數與位移重現以避免任何行為差異。

    for (int a2 = 0; a2 < 6; ++a2) {
        for (int a3 = 0; a3 < 8; ++a3) {
            int v1 = 0;
            for (int k = 0; k < 100000; ++k) {
                // 對齊反編譯：使用 64-bit 乘法與右移
                // v3 = (int)((unsigned __int64)(1374389535i64 * v1) >> 32) >> 5;
                // *((_DWORD*)this - 1) = (v3 >> 31) + v3;
                // (在本問題的 v1 範圍內 v3 >> 31 恒為 0，但仍照抄計算)
                uint64_t mul = 1374389535ull * static_cast<uint32_t>(v1);
                uint32_t v3 = static_cast<uint32_t>((mul >> 32) >> 5);
                uint32_t put = (v3 >> 31) + v3; // 與反編譯一致
                m_tbl[a2][a3][k] = static_cast<int>(put);

                v1 += 110;
            }
        }
    }
}

int cltAttackAtb::AdjustDamage(int a2, int a3, int a4) const
{
    // if (a2 < 0 || a2 >= 6 || a3 < 0 || a3 >= 8) return 0;
    if (a2 < 0 || a2 >= 6 || a3 < 0 || a3 >= 8)
        return 0;

    int v4 = a4;
    // if (a4 >= 99999) v4 = 99999;   // 注意：未處理 a4 < 0（忠於反編譯）
    if (a4 >= 99999)
        v4 = 99999;

    // return this[800000 * a2 + 100000 * a3 + v4];
    // 800000 = 8 * 100000
    return m_tbl[a2][a3][v4];
}

static inline bool ieq(const char* a, const char* b)
{
#ifdef _WIN32
    return _stricmp(a, b) == 0;
#else
    return strcasecmp(a, b) == 0;
#endif
}

int cltAttackAtb::GetAttackAtb(const char* s)
{
    // 完整對齊反編譯的分支與奇特 fallback（未知 → 3/EARTH）
    // result = __stricmp(s,"FIRE");
    // if (result) { ... else { v2 = -(__stricmp(s,"DARK") != 0); LOBYTE(v2) = v2 & 0xFE; result = v2 + 5; } }
    if (!s) return 3; // 與 fallback 一致

    if (ieq(s, "FIRE"))       return 0;
    if (ieq(s, "ICE"))        return 1;
    if (ieq(s, "LIGHTNING"))  return 2;
    if (ieq(s, "EARTH"))      return 3;
    if (ieq(s, "HOLY"))       return 4;

    // 最終分支的位元技巧等價行為：
    //   s=="DARK" → 5
    //   其他       → 3  (因 -1 的 LOBYTE 與 0xFE 相與 → -2；-2 + 5 = 3)
    if (ieq(s, "DARK"))       return 5;
    return 3; // fallback（忠於反編譯）
}

int cltAttackAtb::GetAttackTargetAtb(const char* s)
{
    // 完整對齊反編譯：未知 → 0/MONSTER_ETC；"DRAGON" → 7
    if (!s) return 0;

    if (ieq(s, "MONSTER_ETC"))       return 0;
    if (ieq(s, "MONSTER_FIRE"))      return 1;
    if (ieq(s, "MONSTER_ICE"))       return 2;
    if (ieq(s, "MONSTER_LIGHTNING")) return 3;
    if (ieq(s, "BEAST"))             return 4;
    if (ieq(s, "UNDEAD"))            return 5;
    if (ieq(s, "DEMON"))             return 6;

    // 末段位元技巧等價：
    //   s=="DRAGON" → 7
    //   其他         → 0  (因 -1 的 LOBYTE 與 0xF9 相與 → -7；-7 + 7 = 0)
    if (ieq(s, "DRAGON"))            return 7;
    return 0; // fallback（忠於反編譯）
}
