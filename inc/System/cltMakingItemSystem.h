#pragma once

#include <cstdint>

class cltBaseInventory;
class cltSpecialtySystem;
class cltItemKindInfo;
class cltMakingItemKindInfo;
class cltSpecialtyKindInfo;
class CMofMsg;

class cltMakingItemSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltMakingItemKindInfo* makingItemKindInfo, cltSpecialtyKindInfo* specialtyKindInfo);

    cltMakingItemSystem();
    ~cltMakingItemSystem();

    void Initialize(cltBaseInventory* baseInventory, cltSpecialtySystem* specialtySystem, int acquiredCount, std::uint16_t* acquiredKinds);
    void Free();

    int IsAcquireMakingItem(std::uint16_t makingKind);
    int CanAcquireMakingItem(std::uint16_t makingKind);
    void AcquireMakingItem(std::uint16_t makingKind);
    void FillOutMakingItemInfo(CMofMsg* msg);

    unsigned int CanMakingItem(std::uint16_t makingKind, std::uint16_t tryQty);
    int MakingItem(unsigned int seed, std::uint16_t makingKind, std::uint16_t tryQty, std::uint8_t* changedFlag);
    std::uint16_t GetMaxMakeableItemQty(std::uint16_t makingKind);

    int GetAcquiredMakingItemKinds(std::uint16_t* outKinds);
    int GetMakingItemKinds(unsigned int itemType, std::uint16_t* outKinds);

private:
    int IsValidMakingItem(std::uint16_t makingKind);

private:
    cltBaseInventory* m_baseInventory = nullptr;
    cltSpecialtySystem* m_specialtySystem = nullptr;
    std::uint16_t m_acquiredKinds[1000]{};
    int m_acquiredCount = 0;

    static cltItemKindInfo* m_pclItemKindInfo;
    static cltMakingItemKindInfo* m_pclMakingItemKindInfo;
    static cltSpecialtyKindInfo* m_pclSpecialtyKindInfo;
};
