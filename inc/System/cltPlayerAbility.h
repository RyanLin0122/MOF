#pragma once

#include <cstdint>

class cltCharKindInfo;
class cltItemKindInfo;
class cltLevelSystem;
class cltSkillSystem;
class cltEquipmentSystem;
class cltClassSystem;
class cltUsingItemSystem;
class cltUsingSkillSystem;
class cltWorkingPassiveSkillSystem;
class CPlayerSpirit;
class cltEmblemSystem;
class cltMonsterToleranceSystem;
class cltRestBonusSystem;
class cltPetSystem;
class cltMyItemSystem;

class cltPlayerAbility {
public:
    static void InitializeStaticVariable(cltCharKindInfo* charKindInfo, cltItemKindInfo* itemKindInfo);

    void Initialize(
        cltLevelSystem* levelSystem,
        cltSkillSystem* skillSystem,
        cltEquipmentSystem* equipmentSystem,
        cltClassSystem* classSystem,
        cltUsingItemSystem* usingItemSystem,
        cltUsingSkillSystem* usingSkillSystem,
        CPlayerSpirit* playerSpirit,
        cltEmblemSystem* emblemSystem,
        cltWorkingPassiveSkillSystem* workingPassiveSkillSystem,
        cltMonsterToleranceSystem* monsterToleranceSystem,
        cltRestBonusSystem* restBonusSystem,
        cltPetSystem* petSystem,
        cltMyItemSystem* myItemSystem,
        int hp,
        int mp,
        std::uint16_t str,
        std::uint16_t dex,
        std::uint16_t intel,
        std::uint16_t vit,
        std::uint16_t bonusPoint);
    void Free();

    static void IncreaseBonusPoint(cltPlayerAbility* self, int value);
    static void ResetAbility(cltPlayerAbility* self);
    void DecreaseBonusPoint(std::uint16_t value);

    std::uint16_t GetBaseStr() const;
    std::uint16_t GetBaseDex() const;
    std::uint16_t GetBaseInt() const;
    std::uint16_t GetBaseVit() const;

    void SetMana(int mana);
    int GetMP() const;
    void DecreaseMP(int value);
    std::uint16_t GetBonusPoint() const;

    void IncreaseStr(int value, int byBonusPoint);
    void IncreaseDex(int value, int byBonusPoint);
    void IncreaseInt(int value, int byBonusPoint);
    void IncreaseVit(int value, int byBonusPoint);

    int GetReqBPForIncreaseStat(std::uint16_t baseStat, int amount);

    int GetHitRateAdvantage(std::uint16_t a2, int a3, void* party = nullptr) const;
    int GetHitRate(std::uint16_t a2, void* party = nullptr) const;
    int GetCriticalHitRateAdvantage(std::uint16_t a2) const;
    int GetCriticalHitRate(std::uint16_t a2) const;
    int GetMissRateAdvantage(std::uint16_t a2) const;
    int GetMissRate(std::uint16_t a2) const;
    int GetSkillAPowerAdvantage(std::uint16_t a2) const;
    int GetShopItemPriceAdvangtage() const;
    int GetSellingAgencyTaxAdvantage() const;

    void GetAPower(std::uint16_t* outMin, std::uint16_t* outMax, std::uint16_t a5, std::uint16_t a6, int a7, void* party, int a9, int a10, std::uint16_t* a11, int a12) const;
    int GetDPower(std::uint16_t a2, void* party, int a4) const;

    std::uint16_t GetStr(int a2 = 0, void* party = nullptr) const;
    std::uint16_t GetDex(int a2 = 0, void* party = nullptr) const;
    std::uint16_t GetInt(int a2 = 0, void* party = nullptr) const;
    std::uint16_t GetVit(int a2 = 0, void* party = nullptr) const;

    void IncreaseMP(int value, int a3 = 0, void* party = nullptr);
    void IncreaseMPPercent(int percent, unsigned char a3 = 0, void* party = nullptr);
    void DecreaseMPPercent(int percent, unsigned char a3 = 0, void* party = nullptr);

    int GetHPRate(void* party = nullptr) const;
    int CanMultiAttack() const;
    int GetAttackSpeedAdvantage() const;
    int GetDamageHP2ManaRate() const;
    int GetDropRateAdvantage() const;
    int GetDeadPenaltyExpAdvantage() const;
    int GetExpAdvantage(void* party = nullptr) const;
    int IsActiveFastRun() const;
    int IsActiveNonDelayAttack() const;
    int GetAttackAtb(int a2 = 0) const;

    int GetItemRecoverHPAdvantage() const;
    int GetItemRecoverManaAdvantage() const;
    int GetMagicResistRateAdvantage() const;
    int GetMagicResistRate() const;
    int GetPartyAPowerAdvantage(void* party = nullptr) const;
    int GetPartyDPowerAdvantage(void* party = nullptr) const;
    int GetPartyHitRateAdvantage(void* party = nullptr) const;

    bool GetMaxFaintingInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const;
    bool GetMaxConfusionInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const;
    bool GetMaxFreezingInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const;
    int GetMaxHP(std::uint16_t vit);
    int GetMaxMP(std::uint16_t intel);
    int CanResetAbility() const;
    int GetNeedManaForUsingSkill(int baseSkillMana) const;

    int GetBuffNum() const;
    int GetMaxBuffNum() const;
    bool CanAddBuff() const;

private:
    static cltCharKindInfo* m_pclCharKindInfo;
    static cltItemKindInfo* m_pclItemKindInfo;

    cltLevelSystem* m_pLevelSystem = nullptr;
    cltSkillSystem* m_pSkillSystem = nullptr;
    cltEquipmentSystem* m_pEquipmentSystem = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
    cltUsingItemSystem* m_pUsingItemSystem = nullptr;
    cltUsingSkillSystem* m_pUsingSkillSystem = nullptr;
    cltWorkingPassiveSkillSystem* m_pWorkingPassiveSkillSystem = nullptr;
    CPlayerSpirit* m_pPlayerSpirit = nullptr;
    cltEmblemSystem* m_pEmblemSystem = nullptr;
    cltMonsterToleranceSystem* m_pMonsterToleranceSystem = nullptr;
    cltRestBonusSystem* m_pRestBonusSystem = nullptr;
    cltPetSystem* m_pPetSystem = nullptr;
    cltMyItemSystem* m_pMyItemSystem = nullptr;

    std::uint16_t m_baseStr = 0;
    std::uint16_t m_baseDex = 0;
    std::uint16_t m_baseInt = 0;
    std::uint16_t m_baseVit = 0;
    std::uint16_t m_initialStr = 0;
    std::uint16_t m_initialDex = 0;
    std::uint16_t m_initialInt = 0;
    std::uint16_t m_initialVit = 0;
    std::uint16_t m_bonusPoint = 0;
    int m_hp = 1;
    int m_mana = 0;
    int m_buffNum = 0;
    int m_maxBuffNum = 8;
};
