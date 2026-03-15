#include "System/cltPetKeepingSystem.h"

#include <cstring>

#include "Info/cltPetKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltMoneySystem.h"
#include "System/cltPetInventorySystem.h"
#include "System/cltPetSkillSystem.h"
#include "System/cltPetSystem.h"

cltPetKindInfo* cltPetKeepingSystem::m_pclPetKindInfo = nullptr;

void cltPetKeepingSystem::InitializeStaticVariable(cltPetKindInfo* petKindInfo) { m_pclPetKindInfo = petKindInfo; }

cltPetKeepingSystem::cltPetKeepingSystem() { Free(); }

void cltPetKeepingSystem::Initialize(int nowTime, std::uint16_t* owner, cltPetSystem* petSystem, cltMoneySystem* moneySystem, CMofMsg* msg) {
    moneySystem_ = moneySystem;
    owner_ = owner;
    petSystem_ = petSystem;
    nowTime_ = nowTime;
    keepings_.fill({});

    int count = 0;
    msg->Get_LONG(&count);
    keepingCount_ = count;
    if (count > 20) count = 20;

    if (keepingCount_ <= 0) {
        lock_ = 0;
        return;
    }

    const int loopCount = keepingCount_ < 20 ? keepingCount_ : 20;
    for (int idx = 0; idx < loopCount; ++idx) {
        auto& info = keepings_[idx];
        int itemCount = 0;
        int skillCount = 0;
        msg->Get_LONG(&info.petId);
        msg->Get_WORD(&info.petKind);
        msg->Get_Z1(info.petName, 0, 0, nullptr);
        msg->Get_LONG(&info.petExp);
        msg->Get_LONG(&info.petSatiety);
        msg->Get_BYTE(&info.bagNum);
        msg->Get_LONG(&itemCount);
        for (int i = 0; i < itemCount; ++i) {
            std::uint8_t slot = 0;
            msg->Get_BYTE(&slot);
            msg->Get_WORD(&info.items[slot].kind);
            msg->Get_WORD(&info.items[slot].qty);
        }
        msg->Get_LONG(&skillCount);
        for (int i = 0; i < skillCount; ++i) {
            msg->Get_WORD(&info.skills[i]);
        }
        msg->Get_LONG(&info.keepingStartTime);
    }
    lock_ = 0;
}

void cltPetKeepingSystem::Free() {
    moneySystem_ = nullptr;
    petSystem_ = nullptr;
    owner_ = nullptr;
    nowTime_ = 0;
    lock_ = 0;
    keepings_.fill({});
}

void cltPetKeepingSystem::FillOutPetKeepingInfo(CMofMsg* msg) {
    auto* countPos = reinterpret_cast<std::int32_t*>(msg->GetCurrentPos());
    msg->Put_LONG(0);
    int count = 0;
    for (const auto& info : keepings_) {
        if (!info.petId) continue;
        ++count;
        msg->Put_LONG(info.petId);
        msg->Put_WORD(info.petKind);
        msg->Put_Z1(const_cast<char*>(info.petName));
        msg->Put_LONG(info.petExp);
        msg->Put_LONG(info.petSatiety);
        msg->Put_BYTE(info.bagNum);
        auto* itemCountPos = reinterpret_cast<std::int32_t*>(msg->GetCurrentPos());
        msg->Put_LONG(0);
        int itemCount = 0;
        for (int i = 0; i < 255; ++i) {
            if (!info.items[i].kind) continue;
            msg->Put_BYTE(static_cast<std::uint8_t>(i));
            msg->Put_WORD(info.items[i].kind);
            msg->Put_WORD(info.items[i].qty);
            ++itemCount;
        }
        *itemCountPos = itemCount;
        auto* skillCountPos = reinterpret_cast<std::int32_t*>(msg->GetCurrentPos());
        msg->Put_LONG(0);
        int skillCount = 0;
        for (int i = 0; i < 100; ++i) {
            if (!info.skills[i]) break;
            msg->Put_WORD(info.skills[i]);
            ++skillCount;
        }
        *skillCountPos = skillCount;
        auto* usingSkillCountPos = reinterpret_cast<std::int32_t*>(msg->GetCurrentPos());
        msg->Put_LONG(0);
        int usingSkillCount = 0;
        for (int i = 0; i < 4; ++i) {
            if (!info.usingSkills[i]) continue;
            msg->Put_BYTE(static_cast<std::uint8_t>(i));
            msg->Put_WORD(info.usingSkills[i]);
            ++usingSkillCount;
        }
        *usingSkillCountPos = usingSkillCount;
    }
    *countPos = count;
}

void* cltPetKeepingSystem::GetOwner() { return owner_; }

int cltPetKeepingSystem::CanKeepingPet() {
    if (IsLock() == 1) return 0;
    for (const auto& k : keepings_) if (k.petId == 0) return 1;
    return 0;
}

void cltPetKeepingSystem::KeepingPet(int nowTime) {
    for (auto& k : keepings_) {
        if (k.petId != 0) continue;

        k.petId = petSystem_->GetPetID();
        k.petKind = petSystem_->GetPetKind();
        std::strcpy(k.petName, petSystem_->GetPetName());
        k.petExp = petSystem_->GetPetExp();
        k.petSatiety = petSystem_->GetPetSatiety();

        auto* inv = petSystem_->GetPetInventorySystem();
        k.bagNum = inv ? inv->GetPetBagNum() : 0;
        for (int i = 0; inv && i < 255; ++i) {
            auto* it = inv->GetPetInventoryItem(i);
            if (!it) break;
            k.items[i].kind = it->itemKind;
            k.items[i].qty = it->itemQty;
        }

        auto* skill = petSystem_->GetPetSkillSystem();
        if (skill) {
            const auto num = skill->GetPetSkillNum();
            if (auto* skills = skill->GetPetSkillKind()) {
                std::memcpy(k.skills.data(), skills, static_cast<std::size_t>(num) * 2);
            }
            if (auto* usingSkills = skill->GetPetUsingSkillKind()) {
                k.usingSkills[0] = usingSkills[0];
                k.usingSkills[1] = usingSkills[1];
                k.usingSkills[2] = usingSkills[2];
                k.usingSkills[3] = usingSkills[3];
            }
        }
        k.keepingStartTime = nowTime;
        return;
    }
}

int cltPetKeepingSystem::GetKeepingDay(int petId) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return 0;
    return ((nowTime_ - k->keepingStartTime) / 86400) + 1;
}

int cltPetKeepingSystem::GetKeepingCost(int petId) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return 0;
    auto* kindInfo = m_pclPetKindInfo->GetPetKindInfo(k->petKind);
    if (!kindInfo) return 0;
    return 100 * kindInfo->levelComputed * GetKeepingDay(petId);
}

strKeepingPetInfo* cltPetKeepingSystem::GetKeepingPetInfo(int petId) {
    if (petId == 0) return nullptr;
    for (auto& k : keepings_) if (k.petId == petId) return &k;
    return nullptr;
}

strKeepingPetInfo* cltPetKeepingSystem::GetKeepingPetInfo() { return keepings_.data(); }

int cltPetKeepingSystem::CanReleaseKeepingPet(int petId) {
    if (IsLock() == 1) return 1;
    if (!GetKeepingPetInfo(petId)) return 1;
    auto* release = GetReleaseKeepingPetCost(petId);
    if (!moneySystem_->CanIncreaseMoney(static_cast<int>(reinterpret_cast<std::uintptr_t>(release)))) return 106;
    return IsPetInventoryEmpty(petId) ? 0 : 1900;
}

void cltPetKeepingSystem::ReleaseKeepingPet(int petId, int* outCost) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return;
    auto* reward = GetReleaseKeepingPetCost(petId);
    const int rewardValue = static_cast<int>(reinterpret_cast<std::uintptr_t>(reward));
    moneySystem_->IncreaseMoney(rewardValue);
    *k = {};
    if (outCost) *outCost = rewardValue;
}

strPetKindInfo* cltPetKeepingSystem::GetReleaseKeepingPetCost(int petId) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return nullptr;
    std::uint16_t skillCount = 0;
    for (std::size_t i = 0; i < k->skills.size(); ++i) {
        if (!k->skills[i]) break;
        ++skillCount;
    }
    return m_pclPetKindInfo->GetPetReleaseCost(k->petKind, skillCount);
}

int cltPetKeepingSystem::CanTakeKeepingPet(int petId) {
    if (IsLock() == 1) return 1;
    if (!GetKeepingPetInfo(petId)) return 1;
    if (!moneySystem_->CanDecreaseMoney(GetKeepingCost(petId))) return 1;
    if (petSystem_->GetPetID()) return petSystem_->CanKeepingPet() ? 0 : 1;
    return petSystem_->CanCreatePet() ? 0 : 1;
}

void cltPetKeepingSystem::TakeKeepingPet(int petId, int nowTime) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return;
    *k = {};
    KeepingPet(nowTime);
}

int cltPetKeepingSystem::IsPetInventoryEmpty(int petId) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return 0;
    for (const auto& item : k->items) {
        if (item.kind != 0) return 0;
    }
    return 1;
}

int cltPetKeepingSystem::CanPetMarketRegistry(int petId, int price) {
    if (IsLock() == 1) return 1;
    if (!GetKeepingPetInfo(petId)) return 1;
    const int keepingCost = GetKeepingCost(petId);
    return !moneySystem_->CanDecreaseMoney(price + keepingCost);
}

void cltPetKeepingSystem::PetMarketRegistry(int petId, int price, int tax) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return;
    GetReleaseKeepingPetCost(petId);
    moneySystem_->DecreaseMoney(price + tax);
    *k = {};
}

void cltPetKeepingSystem::Lock() { lock_ = 1; }
void cltPetKeepingSystem::Unlock() { lock_ = 0; }
int cltPetKeepingSystem::IsLock() { return lock_; }
