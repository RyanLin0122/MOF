#include "System/cltPetSystem.h"

#include <algorithm>
#include <cstring>

#include "Info/cltCharKindInfo.h"
#include "Info/cltPetKindInfo.h"
#include "Info/cltPetSkillKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltLevelSystem.h"
#include "System/cltPetInventorySystem.h"
#include "System/cltPetSkillSystem.h"

struct cltTimerManager {
    void SetTimer(int, int);
};
struct cltMoneySystem {
    int CanIncreaseMoney(int);
    int CanDecreaseMoney(int);
    void IncreaseMoney(int);
    void DecreaseMoney(int);
};

cltPetKindInfo* cltPetSystem::m_pclPetKindInfo = nullptr;
cltPetSkillKindInfo* cltPetSystem::m_pclPetSkillKindInfo = nullptr;
cltCharKindInfo* cltPetSystem::m_pclCharKindInfo = nullptr;
cltTimerManager* cltPetSystem::m_pclTimerManager = nullptr;

void cltPetSystem::InitializeStaticVariable(cltPetKindInfo* a1, cltPetSkillKindInfo* a2, cltCharKindInfo* a3, cltTimerManager* a4) {
    m_pclPetKindInfo = a1;
    m_pclPetSkillKindInfo = a2;
    m_pclCharKindInfo = a3;
    m_pclTimerManager = a4;
}

cltPetSystem::cltPetSystem() = default;
cltPetSystem::~cltPetSystem() { Free(); }

void cltPetSystem::Initialize(cltLevelSystem* a2, cltMoneySystem* a3, CMofMsg* a4) {
    levelSystem_ = a2;
    moneySystem_ = a3;
    petID_ = 0;

    if (a4) {
        a4->Get_LONG(&petID_);
    }

    if (petID_) {
        a4->Get_WORD(&petKind_);
        a4->Get_Z1(petName_.data(), 0, 0, nullptr);

        std::uint8_t isActive = 0;
        a4->Get_BYTE(&isActive);
        a4->Get_LONG(&petExp_);
        a4->Get_LONG(&petSatiety_);

        petKindInfo_ = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetKindInfo(petKind_) : nullptr;
        SetActivity(petKindInfo_ ? isActive : 0);

        if (petInventorySystem_) {
            petInventorySystem_->Initialize(a4);
        }
        if (petSkillSystem_) {
            petSkillSystem_->Initialize(a4);
        }
    } else {
        SetActivity(0);
    }
}

void cltPetSystem::Free() {
    SetActivity(0);
    petID_ = 0;
    petKind_ = 0;
    petName_.fill(0);
    petKindInfo_ = nullptr;
    petExp_ = 0;
    petSatiety_ = 0;

    if (petInventorySystem_) {
        petInventorySystem_->Free();
    }
    if (petSkillSystem_) {
        petSkillSystem_->Free();
    }

    levelSystem_ = nullptr;
    moneySystem_ = nullptr;
    lockFlag_ = 0;
    petExpChanged_ = 0;
}

int cltPetSystem::CanCreatePet() {
    if (IsLock() == 1) {
        return 0;
    }
    return petKindInfo_ == nullptr;
}

void cltPetSystem::CreatePet(int a2, std::uint16_t a3) {
    petID_ = a2;
    petKind_ = a3;
    petKindInfo_ = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetKindInfo(a3) : nullptr;
    petExp_ = 0;
    petSatiety_ = petKindInfo_ ? petKindInfo_->satiety : 0;
    SetActivity(1);
    if (petInventorySystem_) {
        const std::uint8_t bag = petKindInfo_ ? petKindInfo_->bagInitCount : 0;
        petInventorySystem_->OnPetCreated(bag);
    }
    if (petSkillSystem_) {
        petSkillSystem_->OnPetCreated();
    }
    petName_.fill(0);
}

int cltPetSystem::GetPetID() { return petID_; }
std::uint16_t cltPetSystem::GetPetKind() { return petKind_; }
std::uint16_t cltPetSystem::GetOriginalPetKind() { return petKindInfo_ ? petKindInfo_->dyeKinds[0] : 0; }

int cltPetSystem::CanReleasePet() {
    if (!petKindInfo_) return 1;
    if (IsLock() == 1) return 1;
    if (!moneySystem_ || !moneySystem_->CanIncreaseMoney(0)) return 106;
    if (!petInventorySystem_ || !petInventorySystem_->IsPetInventoryEmpty()) return 1900;
    return 0;
}

void cltPetSystem::ReleasePet(int* a2) {
    strPetKindInfo* cost = GetPetReleaseCost();
    const int costValue = static_cast<int>(reinterpret_cast<std::uintptr_t>(cost));
    if (a2) {
        *a2 = costValue;
    }
    SetActivity(0);
    petID_ = 0;
    petKind_ = 0;
    petKindInfo_ = nullptr;
    petExp_ = 0;
    petSatiety_ = 0;
    petExpChanged_ = 0;
    if (petInventorySystem_) petInventorySystem_->OnPetDeleted();
    if (petSkillSystem_) petSkillSystem_->OnPetDeleted();
    if (moneySystem_) moneySystem_->IncreaseMoney(costValue);
}

int cltPetSystem::CanKeepingPet() { return petKindInfo_ ? (IsLock() != 1) : 0; }
void cltPetSystem::KeepingPet() {
    SetActivity(0);
    petID_ = 0;
    petKind_ = 0;
    petKindInfo_ = nullptr;
    petExp_ = 0;
    petSatiety_ = 0;
    petExpChanged_ = 0;
    if (petInventorySystem_) petInventorySystem_->OnPetDeleted();
    if (petSkillSystem_) petSkillSystem_->OnPetDeleted();
}

int cltPetSystem::CanSetActivity(int a2) {
    if (isActivity_ == a2) return 0;
    if (a2 != 1) return 1;
    if (!petKindInfo_) return 0;
    const std::uint16_t satietyMax = petKindInfo_->satiety;
    if (!satietyMax) return 1;
    return GetPetSatiety() != 0;
}

void cltPetSystem::SetActivity(int a2) {
    isActivity_ = a2;
}

int cltPetSystem::IsActivity() { return isActivity_; }
int cltPetSystem::GetSkillAPowerAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetSkillAPowerAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetAPowerAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetAPowerAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetDPowerAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetDPowerAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetHitRateAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetHitRateAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetAutoRecoverHPAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetAutoRecoverHPAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetAutoRecoverManaAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetAutoRecoverManaAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::CanPickupItem() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->CanPickupItem(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetDropRateAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetDropRateAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetSTRAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetSTRAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetVITAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetVITAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetDEXAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetDEXAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetINTAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetINTAdvantage(petKindInfo_->basePassiveSkill); }
int cltPetSystem::GetAttackSpeedAdvantage() { return IsPetSkillInvalidated() || !petSkillSystem_ ? 0 : petSkillSystem_->GetAttackSpeedAdvantage(petKindInfo_->basePassiveSkill); }

void cltPetSystem::IncreasePetExp(int a2, std::uint16_t* a3) {
    const int prevExp = petExp_;
    if (a3) *a3 = 0;
    const int threshold = petKindInfo_ ? static_cast<int>(petKindInfo_->loveExp) : 0;
    if (threshold) {
        petExp_ += a2;
        if (petExp_ >= threshold && m_pclPetKindInfo) {
            const std::uint16_t next = m_pclPetKindInfo->GetNextPetKind(petKindInfo_->kind);
            if (a3) *a3 = next;
            petKind_ = next;
            petKindInfo_ = m_pclPetKindInfo->GetPetKindInfo(next);
            petExp_ = 0;
            petSatiety_ = petKindInfo_ ? petKindInfo_->satiety : 0;
        }
    }
    if (!petExpChanged_ && prevExp != petExp_) petExpChanged_ = 1;
}

int cltPetSystem::GetPetExp() {
    petExpChanged_ = 0;
    return petExp_;
}

int cltPetSystem::CanIncreasePetSatiety() {
    if (!petKindInfo_) return 1;
    const std::uint16_t maxSatiety = petKindInfo_->satiety;
    if (!maxSatiety) return 1903;
    return petSatiety_ < static_cast<int>(maxSatiety) ? 0 : 0x771;
}

void cltPetSystem::IncreasePetSatiety(int a2) {
    const int maxSatiety = petKindInfo_ ? petKindInfo_->satiety : 0;
    const int updated = petSatiety_ + a2;
    petSatiety_ = updated >= maxSatiety ? maxSatiety : updated;
}

cltPetSkillSystem* cltPetSystem::GetPetSkillSystem() { return petSkillSystem_; }
cltPetInventorySystem* cltPetSystem::GetPetInventorySystem() { return petInventorySystem_; }

void cltPetSystem::OnDecreasePetSatiety() {
    petSatiety_ -= petKindInfo_ ? static_cast<int>(petKindInfo_->satietyDropPerMin) : 0;
    if (petSatiety_ <= 0) petSatiety_ = 0;
    if (!petSatiety_) SetActivity(0);
}

void cltPetSystem::SetPetSatiety(int a2) {
    petSatiety_ = a2;
    if (!petSatiety_) SetActivity(0);
}
int cltPetSystem::GetPetSatiety() { return petSatiety_; }

int cltPetSystem::CanIncreasePetBagNum() {
    if (!petKindInfo_) return 0;
    if (!petInventorySystem_) return 0;
    if (petInventorySystem_->GetPetBagNum() >= petKindInfo_->bagMaxExpand) return 0;
    return petInventorySystem_->CanIncreasePetBagNum();
}
void cltPetSystem::IncreasePetBagNum() { if (petInventorySystem_) petInventorySystem_->IncreasePetBagNum(); }

int cltPetSystem::GetPetLevel() { return petKindInfo_ ? petKindInfo_->levelComputed : 0; }

int cltPetSystem::CanSetUsingSkill(std::uint8_t a2, std::uint16_t a3) {
    return (petKindInfo_ && petSkillSystem_) ? petSkillSystem_->CanSetPetUsingSkill(a2, a3) : 0;
}

void cltPetSystem::SetUsingSkill(std::uint8_t a2, std::uint16_t a3) {
    if (petSkillSystem_) petSkillSystem_->SetPetUsingSkill(a2, a3);
}

unsigned int cltPetSystem::CanChangePetName(char* a2) {
    if (!petKindInfo_) return 1;
    if (!petKindInfo_->canRename) return 1901;
    if (!a2 || !*a2) return 1;
    return std::strlen(a2) > 8;
}

void cltPetSystem::ChangePetName(char* a2) {
    if (!a2) return;
    std::strcpy(petName_.data(), a2);
}

char* cltPetSystem::GetPetName() { return petName_.data(); }

unsigned int cltPetSystem::CanAddPetSkill(std::uint16_t a2) {
    if (!petKindInfo_) return 1;
    if (!petKindInfo_->skillObtained) return 1902;
    auto* skillKind = m_pclPetSkillKindInfo->GetPetSkillKindInfo(a2);
    if (!skillKind) return 1;
    if (petKindInfo_->levelComputed < skillKind->RequiredLevel) return 1;
    return petSkillSystem_ ? (petSkillSystem_->CanAddPetSkill(a2) == 0) : 1;
}

void cltPetSystem::AddPetSkill(std::uint16_t a2) { if (petSkillSystem_) petSkillSystem_->AddPetSkill(a2); }
int cltPetSystem::IsPetSkillInvalidated() {
    if (!petKindInfo_) return 1;
    if (!IsActivity()) return 1;
    return (10 * petKindInfo_->satiety / 100) > petSatiety_;
}

int cltPetSystem::CanMoveItem(std::uint8_t a2, std::uint8_t a3) {
    return petKindInfo_ && petInventorySystem_ ? petInventorySystem_->CanMoveItem(a2, a3) : 0;
}

void cltPetSystem::MoveItem(std::uint8_t a2, std::uint8_t a3, std::uint8_t* a4) {
    if (petInventorySystem_) petInventorySystem_->MoveItem(a2, a3, a4);
}

void cltPetSystem::OnKillMonster(std::uint16_t a2, int* a3, int a4, int a5) {
    std::uint16_t evolved = 0;
    if (a3) *a3 = 0;
    if (!IsActivity() || !m_pclCharKindInfo || !levelSystem_) return;
    void* charInfo = m_pclCharKindInfo->GetCharKindInfo(a2);
    if (!charInfo) return;

    int diff = static_cast<int>(reinterpret_cast<unsigned char*>(charInfo)[146]) - levelSystem_->GetLevel();
    int expGain = 0;
    if (diff > -5) {
        if (diff <= 0) expGain = 2;
        else if (diff == 1) expGain = 10;
        else if (diff == 2) expGain = 15;
        else if (diff == 3) expGain = 20;
        else if (diff == 4) expGain = 25;
        else if (diff == 5) expGain = 30;
        else if (diff == 6) expGain = 35;
        else expGain = 40;
    }

    if (a4 && expGain > 0) expGain += a4 * expGain / 1000;
    if (a5 > 0) expGain += 300 * expGain / 1000;
    if (!expGain) return;

    IncreasePetExp(expGain, &evolved);
    if (a3) *a3 = evolved ? 1 : 0;
}

strPetKindInfo* cltPetSystem::GetPetReleaseCost() {
    if (!petKindInfo_ || !m_pclPetKindInfo || !petSkillSystem_) return nullptr;
    const std::uint16_t skillNum = petSkillSystem_->GetPetSkillNum();
    return m_pclPetKindInfo->GetPetReleaseCost(petKindInfo_->kind, skillNum);
}

void cltPetSystem::Lock() { lockFlag_ = 1; }
void cltPetSystem::Unlock() { lockFlag_ = 0; }
int cltPetSystem::IsLock() { return lockFlag_; }

void cltPetSystem::OnTakeKeepingPet(int a2, CMofMsg* a3) {
    if (petInventorySystem_) petInventorySystem_->Free();
    if (petSkillSystem_) petSkillSystem_->Free();
    if (moneySystem_) moneySystem_->DecreaseMoney(a2);
    a3->Get_LONG(&petID_);
    a3->Get_WORD(&petKind_);
    a3->Get_Z1(petName_.data(), 0, 0, nullptr);
    std::uint8_t isActive = 0;
    a3->Get_BYTE(&isActive);
    a3->Get_LONG(&petExp_);
    a3->Get_LONG(&petSatiety_);
    petKindInfo_ = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetKindInfo(petKind_) : nullptr;
    SetActivity(isActive);
    if (petInventorySystem_) petInventorySystem_->Initialize(a3);
    if (petSkillSystem_) petSkillSystem_->Initialize(a3);
    petExpChanged_ = 0;
}

int cltPetSystem::CanPetMarketRegistry(int a2) {
    if (IsLock() == 1) return 1;
    if (!GetPetID()) return 1;
    return !moneySystem_->CanDecreaseMoney(a2);
}

void cltPetSystem::PetMarketRegistry(int a2) {
    SetActivity(0);
    petID_ = 0;
    petKind_ = 0;
    petKindInfo_ = nullptr;
    petExp_ = 0;
    petSatiety_ = 0;
    petExpChanged_ = 0;
    if (petInventorySystem_) petInventorySystem_->OnPetDeleted();
    if (petSkillSystem_) petSkillSystem_->OnPetDeleted();
    if (moneySystem_) moneySystem_->DecreaseMoney(a2);
}

int cltPetSystem::CanPetMarketBuy() {
    if (IsLock() == 1) return 1;
    return GetPetID() != 0;
}

void cltPetSystem::PetMarketBuy(CMofMsg* a2, int* a3) {
    int price = 0;
    if (!a2) return;
    a2->Get_LONG(&price);
    a2->Get_LONG(&petID_);
    a2->Get_WORD(&petKind_);
    a2->Get_Z1(petName_.data(), 0, 0, nullptr);
    a2->Get_LONG(&petExp_);
    a2->Get_LONG(&petSatiety_);
    petKindInfo_ = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetKindInfo(petKind_) : nullptr;
    SetActivity(0);
    if (petInventorySystem_) petInventorySystem_->Initialize(a2);
    if (petSkillSystem_) petSkillSystem_->Initialize(a2);
    if (moneySystem_) moneySystem_->DecreaseMoney(price);
    if (a3) *a3 = price;
    petExpChanged_ = 0;
}

int cltPetSystem::GetRegistryTax(int a1) {
    long long v = 50LL * a1 / 1000;
    if (v < 10000) {
        v = 10000;
    }
    return static_cast<int>(v);
}

int cltPetSystem::IsPetExpChanged() { return petExpChanged_; }

void cltPetSystem::DyePet(std::uint16_t a2) {
    auto* dyeInfo = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetDyeKindInfo(a2) : nullptr;
    if (!dyeInfo || !petKindInfo_) return;
    const int idx = static_cast<int>(dyeInfo->dyeIndex);
    const std::uint16_t target = petKindInfo_->dyeKinds[idx];
    petKind_ = target;
    if (m_pclPetKindInfo) {
        petKindInfo_ = m_pclPetKindInfo->GetPetKindInfo(target);
    }
}

int cltPetSystem::CanDyePet(std::uint16_t a2) {
    if (!petID_) return 1;
    if (!a2) return 1;
    auto* dyeInfo = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetDyeKindInfo(a2) : nullptr;
    if (!dyeInfo || !petKindInfo_) return 1;
    const int idx = static_cast<int>(dyeInfo->dyeIndex);
    const std::uint16_t target = petKindInfo_->dyeKinds[idx];
    if (!target) return 114;
    return petKind_ != target ? 0 : 0x71;
}
