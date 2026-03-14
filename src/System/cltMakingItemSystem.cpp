#include "System/cltMakingItemSystem.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "Info/cltItemKindInfo.h"
#include "Info/cltMakingItemKindInfo.h"
#include "Info/cltSpecialtyKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Network/CMofMsg.h"
#include "System/cltSpecialtySystem.h"

cltItemKindInfo* cltMakingItemSystem::m_pclItemKindInfo = nullptr;
cltMakingItemKindInfo* cltMakingItemSystem::m_pclMakingItemKindInfo = nullptr;
cltSpecialtyKindInfo* cltMakingItemSystem::m_pclSpecialtyKindInfo = nullptr;

namespace {
int compareMakingType(const void* lhs, const void* rhs) {
    const auto* l = *static_cast<strMakingItemKindInfo* const*>(lhs);
    const auto* r = *static_cast<strMakingItemKindInfo* const*>(rhs);
    const int diff = l->Category - r->Category;
    if (diff != 0) return diff < 0 ? -1 : 1;
    return 1;
}
} // namespace

void cltMakingItemSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltMakingItemKindInfo* makingItemKindInfo, cltSpecialtyKindInfo* specialtyKindInfo) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclMakingItemKindInfo = makingItemKindInfo;
    m_pclSpecialtyKindInfo = specialtyKindInfo;
}

cltMakingItemSystem::cltMakingItemSystem() { Free(); }
cltMakingItemSystem::~cltMakingItemSystem() { Free(); }

void cltMakingItemSystem::Initialize(cltBaseInventory* baseInventory, cltSpecialtySystem* specialtySystem, int acquiredCount, std::uint16_t* acquiredKinds) {
    m_baseInventory = baseInventory;
    m_specialtySystem = specialtySystem;
    m_acquiredCount = std::clamp(acquiredCount, 0, 1000);
    if (acquiredKinds && m_acquiredCount > 0) {
        std::memcpy(m_acquiredKinds, acquiredKinds, sizeof(std::uint16_t) * m_acquiredCount);
    }
}

void cltMakingItemSystem::Free() {
    m_baseInventory = nullptr;
    m_specialtySystem = nullptr;
    m_acquiredCount = 0;
    std::memset(m_acquiredKinds, 0, sizeof(m_acquiredKinds));
}

int cltMakingItemSystem::IsAcquireMakingItem(std::uint16_t makingKind) {
    std::uint16_t allKinds[1000]{};
    const int count = GetAcquiredMakingItemKinds(allKinds);
    for (int i = 0; i < count; ++i) {
        if (allKinds[i] == makingKind) return 1;
    }
    return 0;
}

int cltMakingItemSystem::CanAcquireMakingItem(std::uint16_t makingKind) {
    if (m_acquiredCount >= 1000) return 0;
    return IsAcquireMakingItem(makingKind) != 1;
}

void cltMakingItemSystem::AcquireMakingItem(std::uint16_t makingKind) {
    if (m_acquiredCount < 1000) {
        m_acquiredKinds[m_acquiredCount++] = makingKind;
    }
}

void cltMakingItemSystem::FillOutMakingItemInfo(CMofMsg* msg) {
    msg->Put_WORD(static_cast<std::uint16_t>(m_acquiredCount));
    msg->Put(reinterpret_cast<char*>(m_acquiredKinds), static_cast<unsigned int>(2 * m_acquiredCount));
}

unsigned int cltMakingItemSystem::CanMakingItem(std::uint16_t makingKind, std::uint16_t tryQty) {
    if (!IsAcquireMakingItem(makingKind)) return 1;
    if (!IsValidMakingItem(makingKind)) return 1;

    auto* info = m_pclMakingItemKindInfo ? m_pclMakingItemKindInfo->GetMakingItemKindInfo(makingKind) : nullptr;
    if (!info || !m_baseInventory) return 1;

    for (int i = 0; i < 10; ++i) {
        const auto ingredientKind = info->Ingredient[i].Kind;
        if (!ingredientKind) {
            return m_baseInventory->CanAddInventoryItem(info->ResultItemID, static_cast<std::uint16_t>(tryQty * info->ProduceCount)) != 0 ? 0x44C : 0;
        }
        if (m_baseInventory->GetInventoryItemQuantity(ingredientKind) < static_cast<int>(tryQty * info->Ingredient[i].Count)) {
            return 1101;
        }
    }
    return m_baseInventory->CanAddInventoryItem(info->ResultItemID, static_cast<std::uint16_t>(tryQty * info->ProduceCount)) != 0 ? 0x44C : 0;
}

int cltMakingItemSystem::MakingItem(unsigned int seed, std::uint16_t makingKind, std::uint16_t tryQty, std::uint8_t* changedFlag) {
    auto* info = m_pclMakingItemKindInfo ? m_pclMakingItemKindInfo->GetMakingItemKindInfo(makingKind) : nullptr;
    if (!info || !m_baseInventory) return 0;

    for (int i = 0; i < 10; ++i) {
        const auto ingredientKind = info->Ingredient[i].Kind;
        if (!ingredientKind) break;
        m_baseInventory->DelInventoryItemKind(ingredientKind, static_cast<std::uint16_t>(tryQty * info->Ingredient[i].Count), nullptr, changedFlag);
    }

    std::srand(seed);
    int successCount = 0;
    for (std::uint16_t i = 0; i < tryQty; ++i) {
        if (std::rand() % 10000 < info->SuccessPermyriad) {
            ++successCount;
        }
    }

    strInventoryItem item{};
    item.itemKind = info->ResultItemID;
    item.itemQty = static_cast<std::uint16_t>(successCount * info->ProduceCount);
    m_baseInventory->AddInventoryItem(&item, changedFlag, nullptr);
    return successCount;
}

std::uint16_t cltMakingItemSystem::GetMaxMakeableItemQty(std::uint16_t makingKind) {
    auto* info = m_pclMakingItemKindInfo ? m_pclMakingItemKindInfo->GetMakingItemKindInfo(makingKind) : nullptr;
    if (!info || !m_baseInventory) return 0;

    std::uint16_t maxQty = 0xFFFF;
    for (int i = 0; i < 10; ++i) {
        const auto ingredientKind = info->Ingredient[i].Kind;
        if (!ingredientKind) break;
        const auto ingredientCount = info->Ingredient[i].Count;
        const auto currentQty = static_cast<std::uint16_t>(m_baseInventory->GetInventoryItemQuantity(ingredientKind));
        const auto q = static_cast<std::uint16_t>(currentQty / ingredientCount);
        if (q < maxQty) maxQty = q;
        if (!maxQty) break;
    }
    return maxQty;
}

int cltMakingItemSystem::GetAcquiredMakingItemKinds(std::uint16_t* outKinds) {
    if (!outKinds) return 0;

    int total = 0;
    std::uint16_t specialtyKinds[30]{};
    const int specialtyCount = m_specialtySystem ? m_specialtySystem->GetAcquiredMakingItemSpecialty(specialtyKinds) : 0;
    for (int i = 0; i < specialtyCount; ++i) {
        auto* info = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(specialtyKinds[i]) : nullptr;
        while (info) {
            for (std::uint16_t skillKind : info->wAcquiredSkillKinds) {
                if (!skillKind) break;
                if (total < 1000) outKinds[total++] = skillKind;
            }
            info = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(info->wRequiredSpecialtyKind) : nullptr;
        }
    }

    for (int i = 0; i < m_acquiredCount && total < 1000; ++i) {
        outKinds[total++] = m_acquiredKinds[i];
    }
    return total;
}

int cltMakingItemSystem::GetMakingItemKinds(unsigned int itemType, std::uint16_t* outKinds) {
    std::uint16_t acquired[1000]{};
    const int acquiredCount = GetAcquiredMakingItemKinds(acquired);
    if (acquiredCount <= 0 || !outKinds) return 0;

    std::vector<strMakingItemKindInfo*> infos(acquiredCount, nullptr);
    for (int i = 0; i < acquiredCount; ++i) {
        infos[i] = m_pclMakingItemKindInfo->GetMakingItemKindInfo(acquired[i]);
    }
    std::qsort(infos.data(), infos.size(), sizeof(strMakingItemKindInfo*), compareMakingType);

    int outCount = 0;
    for (auto* info : infos) {
        if (!info) continue;
        auto* itemInfo = m_pclItemKindInfo ? m_pclItemKindInfo->GetItemKindInfo(info->ResultItemID) : nullptr;
        if (itemInfo && itemInfo->m_wItemType == itemType) {
            outKinds[outCount++] = info->MakingID;
        }
    }
    return outCount;
}

int cltMakingItemSystem::IsValidMakingItem(std::uint16_t makingKind) {
    const auto reqSpecialty = m_pclItemKindInfo ? m_pclItemKindInfo->GetReqSpecialtyKindByMakingItemKind(makingKind) : 0;
    if (!reqSpecialty) return 1;
    return m_specialtySystem ? m_specialtySystem->IsAcquiredSpecialty(reqSpecialty) : 0;
}
