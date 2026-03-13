#pragma once

#include <cstdint>

class cltPetKindInfo;
class cltPetSystem;
class cltMoneySystem;
class CMofMsg;

#pragma pack(push, 1)
struct strPetMarketMySalePetInfo {
    int petID;
    std::uint16_t petKind;
    char petName[34];
    int salePrice;
    int saleTime;
    std::uint8_t state;
    std::uint8_t reserved0;
    std::uint16_t optionData[255][2];
    std::uint16_t skillList[100];
    int reserved1;
};
#pragma pack(pop)
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
