#pragma once

#include <cstddef>
#include <cstdint>

class cltPetKindInfo;
class cltPetSystem;
class cltMoneySystem;
class CMofMsg;

#pragma pack(push, 1)
struct strPetMarketMySalePetInfo {
    // +0x000
    int petID;
    // +0x004
    std::uint16_t petKind;
    // +0x006
    char petName[34];
    // +0x028
    int salePrice;
    // +0x02C
    int saleTime;
    // +0x030
    std::uint8_t optionPageCount;
    // +0x031
    std::uint8_t reserved0;
    // +0x032 .. +0x42D (255 * 2WORD)
    std::uint16_t optionData[255][2];
    // +0x42E .. +0x4F5 (100 * WORD)
    std::uint16_t skillList[100];
    // +0x4F6
    int reserved1;
    // +0x4FA .. +0x4FB
    std::uint16_t tailPadding;
};
#pragma pack(pop)

static_assert(offsetof(strPetMarketMySalePetInfo, petID) == 0x000, "petID offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, petKind) == 0x004, "petKind offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, petName) == 0x006, "petName offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, salePrice) == 0x028, "salePrice offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, saleTime) == 0x02C, "saleTime offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, optionPageCount) == 0x030, "optionPageCount offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, optionData) == 0x032, "optionData offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, skillList) == 0x42E, "skillList offset mismatch");
static_assert(offsetof(strPetMarketMySalePetInfo, reserved1) == 0x4F6, "reserved1 offset mismatch");
static_assert(sizeof(strPetMarketMySalePetInfo) == 0x4FC, "strPetMarketMySalePetInfo size mismatch");

class cltPetMarketMySalePetSystem {
public:
    static void InitializeStaticVariable(cltPetKindInfo* petKindInfo);

    cltPetMarketMySalePetSystem();
    ~cltPetMarketMySalePetSystem() = default;

    void Initialize(void* owner, cltPetSystem* petSystem, cltMoneySystem* moneySystem, CMofMsg* msg);
    void Free();
    void FillOutSalePetInfo(CMofMsg* msg);
    void* GetOwner();
    strPetMarketMySalePetInfo* GetPetMarketMySalePetInfo();
    int CanAcceptMoney();
    void AcceptMoney(int* acceptedMoney);
    int CanRegistryPetCancel(int petID);
    void RegistryPetCancel();
    int GetPetMarketMyMoney();

private:
    cltMoneySystem* moneySystem_ = nullptr;
    cltPetSystem* petSystem_ = nullptr;
    void* owner_ = nullptr;
    strPetMarketMySalePetInfo info_{};
    int myMoney_ = 0;

    static cltPetKindInfo* m_pclPetKindInfo;
};
