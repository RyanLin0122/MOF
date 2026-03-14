#pragma once

#include <array>
#include <cstdint>

class CMofMsg;
class cltMoneySystem;
class cltPetInventorySystem;
class cltPetKindInfo;
class cltPetSkillSystem;
class cltPetSystem;
struct strPetKindInfo;

struct strKeepingPetInfo {
    int petId{};
    std::uint16_t petKind{};
    char petName[32]{};
    int petExp{};
    int petSatiety{};
    std::uint8_t bagNum{};
    std::array<std::uint16_t, 255> itemKinds{};
    std::array<std::uint16_t, 255> itemQtys{};
    std::array<std::uint16_t, 100> skills{};
    std::array<std::uint16_t, 4> usingSkills{};
    int keepingStartTime{};
};

class cltPetKeepingSystem {
public:
    static void InitializeStaticVariable(cltPetKindInfo* petKindInfo);

    cltPetKeepingSystem();
    ~cltPetKeepingSystem() = default;

    void Initialize(int nowTime, std::uint16_t* owner, cltPetSystem* petSystem, cltMoneySystem* moneySystem, CMofMsg* msg);
    void Free();
    void FillOutPetKeepingInfo(CMofMsg* msg);

    void* GetOwner();
    int CanKeepingPet();
    void KeepingPet(int nowTime);

    int GetKeepingDay(int petId);
    int GetKeepingCost(int petId);
    strKeepingPetInfo* GetKeepingPetInfo(int petId);
    strKeepingPetInfo* GetKeepingPetInfo();

    int CanReleaseKeepingPet(int petId);
    void ReleaseKeepingPet(int petId, int* outCost);
    strPetKindInfo* GetReleaseKeepingPetCost(int petId);

    int CanTakeKeepingPet(int petId);
    void TakeKeepingPet(int petId, int nowTime);

    int IsPetInventoryEmpty(int petId);
    int CanPetMarketRegistry(int petId, int price);
    void PetMarketRegistry(int petId, int price, int tax);

    void Lock();
    void Unlock();
    int IsLock();

private:
    cltMoneySystem* moneySystem_{};
    cltPetSystem* petSystem_{};
    std::uint16_t* owner_{};
    int nowTime_{};
    int lock_{};

    std::array<strKeepingPetInfo, 20> keepings_{};

    static cltPetKindInfo* m_pclPetKindInfo;
};
