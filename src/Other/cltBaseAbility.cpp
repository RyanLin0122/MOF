#include "Other/cltBaseAbility.h"

// ---------------------- 反編譯對應：建構 / 解構 ----------------------

cltBaseAbility::cltBaseAbility()
{
    // 在 C++ 中，vtable 指標由編譯器自動設置。
    // 反編譯碼中的 "*(_DWORD *)this = &cltBaseAbility::`vftable`" 由語言機制隱含完成。
}

cltBaseAbility::~cltBaseAbility()
{
    // 反編譯碼把 vftable 設回本類別；C++ 解構期亦會如此處理（語言機制）。
}

// ---------------------- 反編譯對應：Initialize ----------------------

void cltBaseAbility::Initialize(int hp, uint16_t str, uint16_t dex, uint16_t intel, uint16_t vit)
{
    // *((_DWORD *)this + 1) = a2;
    // *((_DWORD *)this + 2) = a2;   （MaxHP 初始等於 HP）
    // *((_WORD  *)this + 6) = a3;   （注意反編譯碼是先寫 a4 再 a3，實際儲存順序見下）
    // *((_WORD  *)this + 7) = a4;
    // *((_WORD  *)this + 8) = a5;
    // *((_WORD  *)this + 9) = a6;
    mHP = hp;
    mMaxHP = hp;
    mStr = str;
    mDex = dex;
    mInt = intel;
    mVit = vit;
}

// ---------------------- 反編譯對應：能力值 Getter ----------------------

uint16_t cltBaseAbility::GetStr() { return mStr; }
uint16_t cltBaseAbility::GetDex() { return mDex; }
uint16_t cltBaseAbility::GetInt() { return mInt; }
uint16_t cltBaseAbility::GetVit() { return mVit; }

// ---------------------- 反編譯對應：HP / MaxHP ----------------------

int cltBaseAbility::GetMaxHP(cltPartySystem* /*a2*/)
{
    // 0055F4B0：return *((_DWORD *)this + 2);
    return mMaxHP;
}

int cltBaseAbility::GetHP()
{
    // 0055F4C0：return *((_DWORD *)this + 1);
    return mHP;
}

// ---------------------- 反編譯對應：GetHPRate ----------------------

int cltBaseAbility::GetHPRate(cltPartySystem* a2)
{
    // 0055F4D0：
    // v3 = vtbl[+24](this, a2);               // 取 MaxHP（虛擬）
    // return 100 * vtbl[+28](this) / v3;      // 100*HP / MaxHP（HP 走虛擬）
    int maxHP = VGetMaxHP(a2);
    int hpNow = VGetHP();
    // 反編譯未做除零防護，為保持一致不加額外保護。
    return 100 * hpNow / maxHP;
}

// ---------------------- 反編譯對應：SetHP / IncreaseHP / DecreaseHP ----------------------

void cltBaseAbility::SetHP(int value)
{
    // 0055F500
    mHP = value;
}

void cltBaseAbility::IncreaseHP(int add, cltPartySystem* a3)
{
    // 0055F510：
    // int maxHP = vtbl[+24](this, a3);
    // if (mHP + add >= maxHP) mHP = maxHP; else mHP = mHP + add;
    int maxHP = VGetMaxHP(a3);
    int cur = mHP;
    mHP = (cur + add >= maxHP) ? maxHP : (cur + add);
}

void cltBaseAbility::DecreaseHP(int sub)
{
    // 0055F540：
    // mHP -= sub; if (mHP <= 0) mHP = 0;
    int v = mHP - sub;
    mHP = (v <= 0) ? 0 : v;
}

// ---------------------- 反編譯對應：Increase/DecreaseHPPercent ----------------------

void cltBaseAbility::IncreaseHPPercent(uint8_t percent, cltPartySystem* a3)
{
    // 0055F560：
    // v5 = vtbl[+24](this, a3);                      // MaxHP
    // vtbl[+40](this, (percent * v5) / 100);         // 以「量」增加 HP（虛擬）
    int maxHP = VGetMaxHP(a3);
    int add = (static_cast<int>(percent) * maxHP) / 100;
    VIncreaseHP(add, a3);
}

void cltBaseAbility::DecreaseHPPercent(uint8_t percent, cltPartySystem* a3)
{
    // 0055F5A0：
    // v5 = vtbl[+24](this, a3);                      // MaxHP
    // vtbl[+44](this, (percent * v5) / 100);         // 以「量」減少 HP（虛擬）
    int maxHP = VGetMaxHP(a3);
    int sub = (static_cast<int>(percent) * maxHP) / 100;
    VDecreaseHP(sub);
}

void cltBaseAbility::IncreaseStr(uint16_t add)
{
    mStr += add;
}

void cltBaseAbility::IncreaseDex(uint16_t add)
{
    mDex += add;
}

void cltBaseAbility::IncreaseInt(uint16_t add)
{
    mInt += add;
}

void cltBaseAbility::IncreaseVit(uint16_t add)
{
    mVit += add;
}

// ---------------------- 反編譯對應：vtable 的虛擬函式實作 ----------------------

int cltBaseAbility::VGetMaxHP(cltPartySystem* party)
{
    // 預設行為即回傳欄位值，保持與非虛擬版一致。
    (void)party;
    return mMaxHP;
}

int cltBaseAbility::VGetHP()
{
    // 預設行為即回傳欄位值，保持與非虛擬版一致。
    return mHP;
}

void cltBaseAbility::VIncreaseHP(int add, cltPartySystem* party)
{
    // 預設走與 IncreaseHP 相同的上限判定路徑，需透過虛擬 MaxHP 取上限。
    IncreaseHP(add, party);
}

void cltBaseAbility::VDecreaseHP(int sub)
{
    // 預設走與 DecreaseHP 相同的路徑。
    DecreaseHP(sub);
}
