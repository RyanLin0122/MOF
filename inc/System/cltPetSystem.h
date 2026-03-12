#pragma once

#include <array>
#include <cstdint>

struct cltPetKindInfo;
struct cltPetSkillKindInfo;
struct cltCharKindInfo;
struct cltTimerManager;
struct cltLevelSystem;
struct cltMoneySystem;
class CMofMsg;
class cltPetSkillSystem;
class cltPetInventorySystem;
struct strPetKindInfo;

class cltPetSystem {
public:
    static void InitializeStaticVariable(cltPetKindInfo*, cltPetSkillKindInfo*, cltCharKindInfo*, cltTimerManager*);

    cltPetSystem();
    ~cltPetSystem();

    void Initialize(cltLevelSystem*, cltMoneySystem*, CMofMsg*);
    void Free();
    int CanCreatePet();
    void CreatePet(int, std::uint16_t);
    int GetPetID();
    std::uint16_t GetPetKind();
    std::uint16_t GetOriginalPetKind();
    int CanReleasePet();
    void ReleasePet(int*);
    int CanKeepingPet();
    void KeepingPet();
    int CanSetActivity(int);
    void SetActivity(int);
    int IsActivity();
    int GetSkillAPowerAdvantage();
    int GetAPowerAdvantage();
    int GetDPowerAdvantage();
    int GetHitRateAdvantage();
    int GetAutoRecoverHPAdvantage();
    int GetAutoRecoverManaAdvantage();
    int CanPickupItem();
    int GetDropRateAdvantage();
    int GetSTRAdvantage();
    int GetVITAdvantage();
    int GetDEXAdvantage();
    int GetINTAdvantage();
    int GetAttackSpeedAdvantage();
    void IncreasePetExp(int, std::uint16_t*);
    int GetPetExp();
    int CanIncreasePetSatiety();
    void IncreasePetSatiety(int);
    cltPetSkillSystem* GetPetSkillSystem();
    cltPetInventorySystem* GetPetInventorySystem();
    void OnDecreasePetSatiety();
    void SetPetSatiety(int);
    int GetPetSatiety();
    int CanIncreasePetBagNum();
    void IncreasePetBagNum();
    int GetPetLevel();
    int CanSetUsingSkill(std::uint8_t, std::uint16_t);
    void SetUsingSkill(std::uint8_t, std::uint16_t);
    unsigned int CanChangePetName(char*);
    void ChangePetName(char*);
    char* GetPetName();
    unsigned int CanAddPetSkill(std::uint16_t);
    void AddPetSkill(std::uint16_t);
    int IsPetSkillInvalidated();
    int CanMoveItem(std::uint8_t, std::uint8_t);
    void MoveItem(std::uint8_t, std::uint8_t, std::uint8_t*);
    void OnKillMonster(std::uint16_t, int*, int, int);
    strPetKindInfo* GetPetReleaseCost();
    void Lock();
    void Unlock();
    int IsLock();
    void OnTakeKeepingPet(int, CMofMsg*);
    int CanPetMarketRegistry(int);
    void PetMarketRegistry(int);
    int CanPetMarketBuy();
    void PetMarketBuy(CMofMsg*, int*);
    static int GetRegistryTax(int);
    int IsPetExpChanged();
    void DyePet(std::uint16_t);
    int CanDyePet(std::uint16_t);

private:
    cltLevelSystem* levelSystem_{};
    cltMoneySystem* moneySystem_{};
    cltPetSkillSystem* petSkillSystem_{};
    cltPetInventorySystem* petInventorySystem_{};

    int petID_{};
    std::uint16_t petKind_{};
    std::uint16_t reserved_{};
    std::array<char, 32> petName_{};

    int petExp_{};
    int petSatiety_{};
    int isActivity_{};
    int lockFlag_{};
    int petExpChanged_{};
    int marketState_{};

    strPetKindInfo* petKindInfo_{};

    static cltPetKindInfo* m_pclPetKindInfo;
    static cltPetSkillKindInfo* m_pclPetSkillKindInfo;
    static cltCharKindInfo* m_pclCharKindInfo;
    static cltTimerManager* m_pclTimerManager;
};
