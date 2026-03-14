#include "System/cltMyItemSystem.h"

#include <algorithm>
#include <cstring>
#include <ctime>

#include "Info/cltCoupleRingKindInfo.h"
#include "Info/cltMyItemKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltMarriageSystem.h"
#include "System/cltQuickSlotSystem.h"

cltMyItemKindInfo* cltMyItemSystem::m_pclMyItemKindInfo = nullptr;
cltCoupleRingKindInfo* cltMyItemSystem::m_pclCoupleRingKindInfo = nullptr;
void (*cltMyItemSystem::m_pMyItemFuncPtr_OnMyItemTimeouted)(void*, strMyItem*) = nullptr;

void cltMyItemSystem::InitializeStaticVariable(cltMyItemKindInfo* myItemKindInfo, cltCoupleRingKindInfo* coupleRingKindInfo,
                                               void (*onMyItemTimeouted)(void*, strMyItem*)) {
    m_pclMyItemKindInfo = myItemKindInfo;
    m_pclCoupleRingKindInfo = coupleRingKindInfo;
    m_pMyItemFuncPtr_OnMyItemTimeouted = onMyItemTimeouted;
}

cltMyItemSystem::cltMyItemSystem() { Free(); }
cltMyItemSystem::~cltMyItemSystem() = default;

void cltMyItemSystem::Initialize(void* owner, cltQuickSlotSystem* quickSlotSystem, cltMarriageSystem* marriageSystem, CMofMsg* msg) {
    m_owner = owner;
    m_quickSlotSystem = quickSlotSystem;
    m_marriageSystem = marriageSystem;
    std::memset(m_myItems.data(), 0, sizeof(m_myItems));
    m_myItemCount = 0;

    if (!msg) return;

    int count = 0;
    msg->Get_LONG(&count);
    if (count < 0 || count > 10) return;

    for (int i = 0; i < count; ++i) {
        std::uint16_t kind = 0;
        int v0 = 0;
        int v1 = 0;
        int v2 = 0;
        msg->Get_WORD(&kind);
        msg->Get_LONG(&v0);
        msg->Get_LONG(&v1);
        msg->Get_LONG(&v2);

        m_myItems[m_myItemCount].wKind = kind;
        m_myItems[m_myItemCount].nValue0 = v0;
        m_myItems[m_myItemCount].nValue1 = v1;
        m_myItems[m_myItemCount].nValue2 = v2;
        ++m_myItemCount;
    }
}

void cltMyItemSystem::Free() {
    m_owner = nullptr;
    m_quickSlotSystem = nullptr;
    m_marriageSystem = nullptr;
    std::memset(m_myItems.data(), 0, sizeof(m_myItems));
    m_myItemCount = 0;
}

unsigned int cltMyItemSystem::CanAddMyItem(std::uint16_t myItemKind) {
    if (m_myItemCount >= 10) return 1;

    auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(myItemKind) : nullptr;
    if (!info) return 1;

    if (!info->dwDuplicate) {
        for (int i = 0; i < m_myItemCount; ++i) {
            const auto existingType = m_pclMyItemKindInfo->GetMyItemType(m_myItems[i].wKind);
            if (!existingType || existingType == info->bType) return 1;
            if (m_myItems[i].wKind == myItemKind) return 102;
        }
    }

    if (static_cast<int>(info->dwSpouseDailySummonCharge) > 0) {
        if (!m_marriageSystem || !m_marriageSystem->IsMarried()) return 1;
        const auto coupleRingKind = m_marriageSystem->GetCoupleRingKind();
        auto* ring = m_pclCoupleRingKindInfo ? m_pclCoupleRingKindInfo->GetCoupleRingKindInfo(coupleRingKind) : nullptr;
        if (!ring || ring->canSummonSpouse <= 0) return 1;
    }

    return 0;
}

void cltMyItemSystem::AddMyItem(std::uint16_t myItemKind, int v0, int v1, int v2) {
    if (m_myItemCount >= 10) return;

    auto& item = m_myItems[m_myItemCount++];
    item.wKind = myItemKind;
    item.nValue0 = v0;
    item.nValue1 = v1;
    item.nValue2 = v2;

    auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(myItemKind) : nullptr;
    if (!info) return;

    if (info->dwIsPremiumQuickSlot == 1 && m_quickSlotSystem) {
        m_quickSlotSystem->OnPremiumQuickSlotEnabled();
    }
    if (static_cast<int>(info->dwSpouseDailySummonCharge) > 0 && m_marriageSystem) {
        m_marriageSystem->OnChargeRecallQtyByMyItem(static_cast<int>(info->dwSpouseDailySummonCharge));
    }
}

void cltMyItemSystem::DelMyItem(std::uint16_t myItemKind, int v0) {
    for (int i = 0; i < m_myItemCount; ++i) {
        if (m_myItems[i].wKind == myItemKind && m_myItems[i].nValue0 == v0) {
            std::memmove(&m_myItems[i], &m_myItems[i + 1], sizeof(strMyItem) * (m_myItemCount - i - 1));
            --m_myItemCount;

            auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(myItemKind) : nullptr;
            if (info && info->dwIsPremiumQuickSlot == 1 && m_quickSlotSystem) {
                m_quickSlotSystem->OnPremiumQuickSlotDisabled();
            }
            return;
        }
    }
}

void cltMyItemSystem::DeleteExpiredMyItem() {
    std::time_t now = std::time(nullptr);

    for (int i = 0; i < m_myItemCount; ++i) {
        if (m_myItems[i].nValue0 <= static_cast<int>(now)) {
            auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(m_myItems[i].wKind) : nullptr;
            if (info && info->dwIsPremiumQuickSlot == 1 && m_quickSlotSystem) {
                m_quickSlotSystem->OnPremiumQuickSlotDisabled();
            }
            if (m_pMyItemFuncPtr_OnMyItemTimeouted) {
                m_pMyItemFuncPtr_OnMyItemTimeouted(m_owner, &m_myItems[i]);
            }

            std::memmove(&m_myItems[i], &m_myItems[i + 1], sizeof(strMyItem) * (m_myItemCount - i - 1));
            --m_myItemCount;
            --i;
        }
    }
}

int cltMyItemSystem::GetExpAdvantage() {
    int total = 0;
    for (int i = 0; i < m_myItemCount; ++i) {
        auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(m_myItems[i].wKind) : nullptr;
        if (info) total += static_cast<int>(info->dwExpAdv100);
    }
    return total;
}

int cltMyItemSystem::GetDropMoneyAmountAdvangae() {
    int total = 0;
    for (int i = 0; i < m_myItemCount; ++i) {
        auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(m_myItems[i].wKind) : nullptr;
        if (info) total += static_cast<int>(info->dwDropMoneyAmountAdv100);
    }
    return total;
}

int cltMyItemSystem::GetSpouseChargeRecallQty(cltMyItemSystem* self) { return self ? self->GetSpouseChargeRecallQty() : 0; }

int cltMyItemSystem::GetSpouseChargeRecallQty() {
    for (int i = 0; i < m_myItemCount; ++i) {
        auto* info = m_pclMyItemKindInfo ? m_pclMyItemKindInfo->GetMyItemKindInfo(m_myItems[i].wKind) : nullptr;
        if (info && info->dwSpouseDailySummonCharge) {
            return static_cast<int>(info->dwSpouseDailySummonCharge);
        }
    }
    return 0;
}

int cltMyItemSystem::GetMyItemNum() { return m_myItemCount; }
strMyItem* cltMyItemSystem::GetMyItem() { return m_myItems.data(); }
