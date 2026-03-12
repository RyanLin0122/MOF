#include "System/cltPetSystem.h"

#include <algorithm>
#include <cstring>

struct strPetDyeKindInfo;
struct cltPetKindInfo {
    strPetKindInfo* GetPetKindInfo(std::uint16_t);
    strPetDyeKindInfo* GetPetDyeKindInfo(std::uint16_t);
};
struct cltPetSkillKindInfo {};
struct cltCharKindInfo {};
struct cltTimerManager {
    void SetTimer(int, int);
};
struct cltMoneySystem {
    int CanDecreaseMoney(int);
    void DecreaseMoney(int);
};
struct CMofMsg {
    static void Get_LONG(CMofMsg*, int*);
    static void Get_WORD(CMofMsg*, std::uint16_t*);
    static void Get_Z1(CMofMsg*, char*, int, int, int);
    static void Get_BYTE(CMofMsg*, std::uint8_t*);
};
struct cltPetSkillSystem {
    void Initialize(CMofMsg*);
    void Free();
    void OnPetCreated();
    void OnPetDeleted();
    int GetSkillAPowerAdvantage();
    int CanSetUsingSkill(std::uint8_t, std::uint16_t);
    void SetUsingSkill(std::uint8_t, std::uint16_t);
    unsigned int CanAddPetSkill(std::uint16_t);
    void AddPetSkill(std::uint16_t);
    int IsPetSkillInvalidated();
    void OnKillMonster(std::uint16_t, int*, int, int);
};
struct cltPetInventorySystem {
    void Initialize(CMofMsg*);
    void Free();
    void OnPetCreated(std::uint8_t);
    void OnPetDeleted();
    int CanMoveItem(std::uint8_t, std::uint8_t);
    void MoveItem(std::uint8_t, std::uint8_t, std::uint8_t*);
    int CanIncreasePetBagNum();
    void IncreasePetBagNum();
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
        CMofMsg::Get_LONG(a4, &petID_);
    }

    if (petID_) {
        CMofMsg::Get_WORD(a4, &petKind_);
        CMofMsg::Get_Z1(a4, petName_.data(), 0, 0, 0);

        std::uint8_t isActive = 0;
        CMofMsg::Get_BYTE(a4, &isActive);
        CMofMsg::Get_LONG(a4, &petExp_);
        CMofMsg::Get_LONG(a4, &petSatiety_);

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
    petSatiety_ = 0;
    SetActivity(1);
    if (petInventorySystem_) {
        petInventorySystem_->OnPetCreated(0);
    }
    if (petSkillSystem_) {
        petSkillSystem_->OnPetCreated();
    }
    petName_.fill(0);
}

int cltPetSystem::GetPetID() { return petID_; }
std::uint16_t cltPetSystem::GetPetKind() { return petKind_; }
std::uint16_t cltPetSystem::GetOriginalPetKind() { return petKind_; }

int cltPetSystem::CanReleasePet() {
    if (IsLock() == 1) {
        return 1;
    }
    return petID_ ? 0 : 1;
}

void cltPetSystem::ReleasePet(int* a2) {
    if (a2) {
        *a2 = petID_;
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
}

int cltPetSystem::CanKeepingPet() { return petID_ ? 0 : 1; }
void cltPetSystem::KeepingPet() { SetActivity(0); }

int cltPetSystem::CanSetActivity(int a2) {
    if (!petID_) return 1;
    if (!petKindInfo_) return 1;
    return isActivity_ == a2 ? 1 : 0;
}

void cltPetSystem::SetActivity(int a2) {
    isActivity_ = a2;
    if (isActivity_ && m_pclTimerManager) {
        m_pclTimerManager->SetTimer(6, 30000);
    }
}

int cltPetSystem::IsActivity() { return isActivity_; }
int cltPetSystem::GetSkillAPowerAdvantage() { return petSkillSystem_ ? petSkillSystem_->GetSkillAPowerAdvantage() : 0; }
int cltPetSystem::GetAPowerAdvantage() { return isActivity_ ? 50 : 0; }
int cltPetSystem::GetDPowerAdvantage() { return isActivity_ ? 50 : 0; }
int cltPetSystem::GetHitRateAdvantage() { return isActivity_ ? 30 : 0; }
int cltPetSystem::GetAutoRecoverHPAdvantage() { return isActivity_ ? 10 : 0; }
int cltPetSystem::GetAutoRecoverManaAdvantage() { return isActivity_ ? 10 : 0; }
int cltPetSystem::CanPickupItem() { return isActivity_ ? 1 : 0; }
int cltPetSystem::GetDropRateAdvantage() { return isActivity_ ? 50 : 0; }
int cltPetSystem::GetSTRAdvantage() { return isActivity_ ? 5 : 0; }
int cltPetSystem::GetVITAdvantage() { return isActivity_ ? 5 : 0; }
int cltPetSystem::GetDEXAdvantage() { return isActivity_ ? 5 : 0; }
int cltPetSystem::GetINTAdvantage() { return isActivity_ ? 5 : 0; }
int cltPetSystem::GetAttackSpeedAdvantage() { return isActivity_ ? 30 : 0; }

void cltPetSystem::IncreasePetExp(int a2, std::uint16_t* a3) {
    if (!petID_ || !isActivity_) return;
    petExp_ += std::max(0, a2);
    petExpChanged_ = 1;
    if (a3) {
        *a3 = static_cast<std::uint16_t>(petExp_ > 0xFFFF ? 0xFFFF : petExp_);
    }
}

int cltPetSystem::GetPetExp() { return petExp_; }

int cltPetSystem::CanIncreasePetSatiety() {
    if (!petID_) return 1;
    return petSatiety_ >= 10000 ? 1 : 0;
}

void cltPetSystem::IncreasePetSatiety(int a2) {
    petSatiety_ += a2;
    if (petSatiety_ > 10000) petSatiety_ = 10000;
}

cltPetSkillSystem* cltPetSystem::GetPetSkillSystem() { return petSkillSystem_; }
cltPetInventorySystem* cltPetSystem::GetPetInventorySystem() { return petInventorySystem_; }

void cltPetSystem::OnDecreasePetSatiety() {
    if (!isActivity_) return;
    if (petSatiety_ > 0) --petSatiety_;
    if (petSatiety_ <= 0) SetActivity(0);
}

void cltPetSystem::SetPetSatiety(int a2) { petSatiety_ = std::clamp(a2, 0, 10000); }
int cltPetSystem::GetPetSatiety() { return petSatiety_; }

int cltPetSystem::CanIncreasePetBagNum() { return petInventorySystem_ ? (petInventorySystem_->CanIncreasePetBagNum() ? 0 : 1) : 1; }
void cltPetSystem::IncreasePetBagNum() { if (petInventorySystem_) petInventorySystem_->IncreasePetBagNum(); }

int cltPetSystem::GetPetLevel() { return petExp_ / 1000 + 1; }

int cltPetSystem::CanSetUsingSkill(std::uint8_t a2, std::uint16_t a3) {
    return petSkillSystem_ ? petSkillSystem_->CanSetUsingSkill(a2, a3) : 1;
}

void cltPetSystem::SetUsingSkill(std::uint8_t a2, std::uint16_t a3) {
    if (petSkillSystem_) petSkillSystem_->SetUsingSkill(a2, a3);
}

unsigned int cltPetSystem::CanChangePetName(char* a2) {
    if (!petID_) return 1;
    if (!a2 || !*a2) return 1;
    return 0;
}

void cltPetSystem::ChangePetName(char* a2) {
    if (!a2) return;
    std::strncpy(petName_.data(), a2, petName_.size());
    petName_[petName_.size() - 1] = '\0';
}

char* cltPetSystem::GetPetName() { return petName_.data(); }

unsigned int cltPetSystem::CanAddPetSkill(std::uint16_t a2) {
    return petSkillSystem_ ? petSkillSystem_->CanAddPetSkill(a2) : 1;
}

void cltPetSystem::AddPetSkill(std::uint16_t a2) { if (petSkillSystem_) petSkillSystem_->AddPetSkill(a2); }
int cltPetSystem::IsPetSkillInvalidated() { return petSkillSystem_ ? petSkillSystem_->IsPetSkillInvalidated() : 0; }

int cltPetSystem::CanMoveItem(std::uint8_t a2, std::uint8_t a3) {
    return petInventorySystem_ ? petInventorySystem_->CanMoveItem(a2, a3) : 1;
}

void cltPetSystem::MoveItem(std::uint8_t a2, std::uint8_t a3, std::uint8_t* a4) {
    if (petInventorySystem_) petInventorySystem_->MoveItem(a2, a3, a4);
}

void cltPetSystem::OnKillMonster(std::uint16_t a2, int* a3, int a4, int a5) {
    if (petSkillSystem_) {
        petSkillSystem_->OnKillMonster(a2, a3, a4, a5);
    }
}

strPetKindInfo* cltPetSystem::GetPetReleaseCost() { return petKindInfo_; }

void cltPetSystem::Lock() { lockFlag_ = 1; }
void cltPetSystem::Unlock() { lockFlag_ = 0; }
int cltPetSystem::IsLock() { return lockFlag_; }

void cltPetSystem::OnTakeKeepingPet(int, CMofMsg* a3) {
    SetActivity(0);
    if (a3) {
        CMofMsg::Get_LONG(a3, &petID_);
        CMofMsg::Get_WORD(a3, &petKind_);
        CMofMsg::Get_Z1(a3, petName_.data(), 0, 0, 0);
        CMofMsg::Get_LONG(a3, &petExp_);
        CMofMsg::Get_LONG(a3, &petSatiety_);
        petKindInfo_ = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetKindInfo(petKind_) : nullptr;
    }
}

int cltPetSystem::CanPetMarketRegistry(int a2) {
    if (IsLock() == 1) return 1;
    if (!GetPetID()) return 1;
    return moneySystem_ ? !moneySystem_->CanDecreaseMoney(a2) : 1;
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
    CMofMsg::Get_LONG(a2, &price);
    CMofMsg::Get_LONG(a2, &petID_);
    CMofMsg::Get_WORD(a2, &petKind_);
    CMofMsg::Get_Z1(a2, petName_.data(), 0, 0, 0);
    CMofMsg::Get_LONG(a2, &petExp_);
    CMofMsg::Get_LONG(a2, &petSatiety_);
    petKindInfo_ = m_pclPetKindInfo ? m_pclPetKindInfo->GetPetKindInfo(petKind_) : nullptr;
    SetActivity(0);
    if (petInventorySystem_) petInventorySystem_->Initialize(a2);
    if (petSkillSystem_) petSkillSystem_->Initialize(a2);
    if (moneySystem_) moneySystem_->DecreaseMoney(price);
    if (a3) *a3 = price;
    marketState_ = 0;
}

int cltPetSystem::GetRegistryTax(int a1) {
    long long v = 50LL * a1 / 1000;
    if (v < 10000) {
        v = 10000;
    }
    return static_cast<int>(v);
}

int cltPetSystem::IsPetExpChanged() { return petExpChanged_; }

void cltPetSystem::DyePet(std::uint16_t) {
    // 需要 cltPetKindInfo/strPetDyeKindInfo 完整版型才可完全等價。
}

int cltPetSystem::CanDyePet(std::uint16_t a2) {
    if (!petID_) return 1;
    if (!a2) return 1;
    return 0;
}
