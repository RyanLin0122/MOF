#ifndef CLT_BASE_ABILITY_H
#define CLT_BASE_ABILITY_H

#include <cstdint>

// 前置宣告：與反編譯碼一致
struct cltPartySystem;

class cltBaseAbility
{
public:
    // 對應 0055F3F0：建構子只設定 vftable（在 C++ 中自然完成）
    cltBaseAbility();

    // 對應 0055F420：虛擬解構子
    virtual ~cltBaseAbility();

    // 對應 0055F430：Initialize
    void Initialize(int hp, uint16_t str, uint16_t dex, uint16_t intel, uint16_t vit);

    // 對應 0055F470 ~ 0055F4A0：四項能力值 Getter
    uint16_t GetStr();
    uint16_t GetDex();
    uint16_t GetInt();
    uint16_t GetVit();

    // 對應 0055F4B0：GetMaxHP（非虛擬版，忽略 a2）
    int GetMaxHP(cltPartySystem* /*a2*/);

    // 對應 0055F4C0：GetHP（非虛擬版）
    int GetHP();

    // 對應 0055F4D0：GetHPRate（使用 vtable 的虛擬呼叫）
    int GetHPRate(cltPartySystem* a2);

    // 對應 0055F500：SetHP
    void SetHP(int value);

    // 對應 0055F510：IncreaseHP（帶隊伍，需以虛擬 GetMaxHP 取上限）
    void IncreaseHP(int add, cltPartySystem* a3);

    // 對應 0055F540：DecreaseHP
    void DecreaseHP(int sub);

    // 對應 0055F560：IncreaseHPPercent
    void IncreaseHPPercent(uint8_t percent, cltPartySystem* a3);

    // 對應 0055F5A0：DecreaseHPPercent
    void DecreaseHPPercent(uint8_t percent, cltPartySystem* a3);

    // 對應 0055F5E0：IncreaseStr
    void IncreaseStr(uint16_t add);

    // 對應 0055F5F0：IncreaseDex
    void IncreaseDex(uint16_t add);

    // 對應 0055F600：IncreaseInt
    void IncreaseInt(uint16_t add);

    // 對應 0055F610：IncreaseVit
    void IncreaseVit(uint16_t add);

    // ==== 重要：對應 vtable +24, +28, +40, +44 的虛擬函式 ====
    // vtable +24：取得 MaxHP（帶 cltPartySystem*）
    virtual int VGetMaxHP(cltPartySystem* party);

    // vtable +28：取得 HP（無參數）
    virtual int VGetHP();

    // vtable +40：增加 HP（以量增加，帶 cltPartySystem*）
    virtual void VIncreaseHP(int add, cltPartySystem* party);

    // vtable +44：減少 HP（以量減少）
    virtual void VDecreaseHP(int sub);

private:
    // 依反編譯的存取偏移量配置欄位順序：
    // [vptr]
    // +4  : mHP              => *((_DWORD*)this + 1)
    // +8  : mMaxHP           => *((_DWORD*)this + 2)
    // +12 : mStr (word idx6) => *((_WORD*)this + 6)
    // +14 : mDex (word idx7) => *((_WORD*)this + 7)
    // +16 : mInt (word idx8) => *((_WORD*)this + 8)
    // +18 : mVit (word idx9) => *((_WORD*)this + 9)
    int      mHP{ 0 };
    int      mMaxHP{ 0 };
    uint16_t mStr{ 0 };
    uint16_t mDex{ 0 };
    uint16_t mInt{ 0 };
    uint16_t mVit{ 0 };
};

#endif // CLT_BASE_ABILITY_H
