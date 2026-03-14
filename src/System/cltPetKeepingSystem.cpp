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
    Free();
    nowTime_ = nowTime;
    owner_ = owner;
    petSystem_ = petSystem;
    moneySystem_ = moneySystem;

    if (!msg) return;

    int count = 0;
    msg->Get_LONG(&count);
    count = std::min(count, static_cast<int>(keepings_.size()));

    for (int idx = 0; idx < count; ++idx) {
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
            if (slot >= info.itemKinds.size()) {
                std::uint16_t dummy = 0;
                msg->Get_WORD(&dummy);
                msg->Get_WORD(&dummy);
                continue;
            }
            msg->Get_WORD(&info.itemKinds[slot]);
            msg->Get_WORD(&info.itemQtys[slot]);
        }
        msg->Get_LONG(&skillCount);
        for (int i = 0; i < skillCount && i < static_cast<int>(info.skills.size()); ++i) {
            msg->Get_WORD(&info.skills[i]);
        }
        msg->Get_LONG(&info.keepingStartTime);
    }
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
    if (!msg) return;
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
            if (!info.itemKinds[i]) continue;
            msg->Put_BYTE(static_cast<std::uint8_t>(i));
            msg->Put_WORD(info.itemKinds[i]);
            msg->Put_WORD(info.itemQtys[i]);
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
        msg->Put_LONG(info.keepingStartTime);
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
    if (!petSystem_) return;
    for (auto& k : keepings_) {
        if (k.petId != 0) continue;

        k.petId = petSystem_->GetPetID();
        k.petKind = petSystem_->GetPetKind();
        std::strncpy(k.petName, petSystem_->GetPetName(), sizeof(k.petName) - 1);
        k.petExp = petSystem_->GetPetExp();
        k.petSatiety = petSystem_->GetPetSatiety();

        cltPetInventorySystem* inv = petSystem_->GetPetInventorySystem();
        k.bagNum = inv ? inv->GetPetBagNum() : 0;
        for (int i = 0; inv && i < 255; ++i) {
            if (auto* it = inv->GetPetInventoryItem(i)) {
                k.itemKinds[i] = it->itemKind;
                k.itemQtys[i] = it->itemQty;
            }
        }

        cltPetSkillSystem* skill = petSystem_->GetPetSkillSystem();
        if (skill) {
            const auto num = skill->GetPetSkillNum();
            for (std::uint16_t i = 0; i < num && i < k.skills.size(); ++i) {
                k.skills[i] = 0; // no accessor in current codebase
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

    int level = 1;
    if (m_pclPetKindInfo) {
        if (auto* info = m_pclPetKindInfo->GetPetKindInfo(k->petKind)) level = info->levelComputed;
    }
    return 100 * level * GetKeepingDay(petId);
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
    if (!moneySystem_ || !moneySystem_->CanIncreaseMoney(static_cast<int>(reinterpret_cast<std::uintptr_t>(release)))) return 106;
    return IsPetInventoryEmpty(petId) ? 0 : 1900;
}

void cltPetKeepingSystem::ReleaseKeepingPet(int petId, int* outCost) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return;

    auto* reward = GetReleaseKeepingPetCost(petId);
    const int rewardValue = static_cast<int>(reinterpret_cast<std::uintptr_t>(reward));
    if (moneySystem_) moneySystem_->IncreaseMoney(rewardValue);
    *k = {};
    if (outCost) *outCost = rewardValue;
}

strPetKindInfo* cltPetKeepingSystem::GetReleaseKeepingPetCost(int petId) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k || !m_pclPetKindInfo) return nullptr;

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
    if (!moneySystem_ || !moneySystem_->CanDecreaseMoney(GetKeepingCost(petId))) return 1;

    if (!petSystem_) return 1;
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
    for (std::size_t i = 0; i < k->itemKinds.size(); ++i) {
        if (k->itemKinds[i] != 0) return 0;
    }
    return 1;
}

int cltPetKeepingSystem::CanPetMarketRegistry(int petId, int price) {
    if (IsLock() == 1) return 1;
    if (!GetKeepingPetInfo(petId)) return 1;
    const int keepingCost = GetKeepingCost(petId);
    return !moneySystem_ || !moneySystem_->CanDecreaseMoney(price + keepingCost);
}

void cltPetKeepingSystem::PetMarketRegistry(int petId, int price, int tax) {
    auto* k = GetKeepingPetInfo(petId);
    if (!k) return;
    if (moneySystem_) moneySystem_->DecreaseMoney(price + tax);
    *k = {};
}

void cltPetKeepingSystem::Lock() { lock_ = 1; }
void cltPetKeepingSystem::Unlock() { lock_ = 0; }
int cltPetKeepingSystem::IsLock() { return lock_; }
