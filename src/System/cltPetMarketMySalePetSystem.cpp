#include "System/cltPetMarketMySalePetSystem.h"

#include <cstring>

#include "Info/cltPetKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltMoneySystem.h"

cltPetKindInfo* cltPetMarketMySalePetSystem::m_pclPetKindInfo = nullptr;

void cltPetMarketMySalePetSystem::InitializeStaticVariable(cltPetKindInfo* petKindInfo) { m_pclPetKindInfo = petKindInfo; }

cltPetMarketMySalePetSystem::cltPetMarketMySalePetSystem() = default;

void cltPetMarketMySalePetSystem::Initialize(void* owner, cltPetSystem* petSystem, cltMoneySystem* moneySystem, CMofMsg* msg) {
    petSystem_ = petSystem;
    owner_ = owner;
    moneySystem_ = moneySystem;
    std::memset(&info_, 0, sizeof(info_));

    int optionCount = 0;
    int skillCount = 0;
    std::uint8_t optionIndex = 0;

    msg->Get_LONG(&info_.petID);
    msg->Get_WORD(&info_.petKind);
    msg->Get_Z1(info_.petName, 0, 0, nullptr);
    msg->Get_LONG(&info_.salePrice);
    msg->Get_LONG(&info_.saleTime);
    msg->Get_BYTE(&info_.state);

    msg->Get_LONG(&optionCount);
    for (int i = 0; i < optionCount; ++i) {
        msg->Get_BYTE(&optionIndex);
        msg->Get_WORD(&info_.optionData[optionIndex][0]);
        msg->Get_WORD(&info_.optionData[optionIndex][1]);
    }

    msg->Get_LONG(&skillCount);
    for (int i = 0; i < skillCount; ++i) {
        msg->Get_WORD(&info_.skillList[i]);
    }

    msg->Get_LONG(&info_.reserved1);
    msg->Get_LONG(&myMoney_);
}

void cltPetMarketMySalePetSystem::Free() {
    owner_ = nullptr;
    moneySystem_ = nullptr;
    petSystem_ = nullptr;
    std::memset(&info_, 0, sizeof(info_));
    myMoney_ = 0;
}

void cltPetMarketMySalePetSystem::FillOutSalePetInfo(CMofMsg* msg) {
    msg->Put_LONG(info_.petID);
    msg->Put_WORD(info_.petKind);
    msg->Put_Z1(info_.petName);
    msg->Put_LONG(info_.salePrice);
    msg->Put_LONG(info_.saleTime);
    msg->Put_BYTE(info_.state);

    std::uint8_t* optionCountPos = msg->GetCurrentPos();
    msg->Put_LONG(0);
    for (int i = 0; i < 255; ++i) {
        if (info_.optionData[i][0]) {
            msg->Put_BYTE(static_cast<std::uint8_t>(i));
            msg->Put_WORD(info_.optionData[i][0]);
            msg->Put_WORD(info_.optionData[i][1]);
            ++*reinterpret_cast<std::uint32_t*>(optionCountPos);
        }
    }

    std::uint8_t* skillCountPos = msg->GetCurrentPos();
    msg->Put_LONG(0);
    for (int i = 0; i < 100; ++i) {
        if (!info_.skillList[i]) break;
        msg->Put_WORD(info_.skillList[i]);
        ++*reinterpret_cast<std::uint32_t*>(skillCountPos);
    }
}

void* cltPetMarketMySalePetSystem::GetOwner() { return owner_; }
strPetMarketMySalePetInfo* cltPetMarketMySalePetSystem::GetPetMarketMySalePetInfo() { return &info_; }

int cltPetMarketMySalePetSystem::CanAcceptMoney() {
    if (!myMoney_) return 1;
    return 106 - (moneySystem_->CanIncreaseMoney(myMoney_) & 0x96);
}

void cltPetMarketMySalePetSystem::AcceptMoney(int* acceptedMoney) {
    *acceptedMoney = myMoney_;
    moneySystem_->IncreaseMoney(myMoney_);
    myMoney_ = 0;
}

int cltPetMarketMySalePetSystem::CanRegistryPetCancel(int petID) { return info_.petID != petID; }
void cltPetMarketMySalePetSystem::RegistryPetCancel() { std::memset(&info_, 0, sizeof(info_)); }
int cltPetMarketMySalePetSystem::GetPetMarketMyMoney() { return myMoney_; }
