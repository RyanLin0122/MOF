#include "System/cltMarriageSystem.h"

#include <ctime>
#include <cstring>

#include "Info/cltCoupleRingKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltWeddingHallKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "Network/CMofMsg.h"

class cltMyItemSystem {
public:
    static int GetSpouseChargeRecallQty(cltMyItemSystem* self);
};

int cltMyItemSystem::GetSpouseChargeRecallQty(cltMyItemSystem*) {
    return 0;
}

cltItemKindInfo* cltMarriageSystem::m_pclItemKindInfo = nullptr;
cltWeddingHallKindInfo* cltMarriageSystem::m_pclWeddingHallKindInfo = nullptr;
cltCoupleRingKindInfo* cltMarriageSystem::m_pclCoupleRingKindInfo = nullptr;
void (*cltMarriageSystem::m_pChargedRecallQtyByMyItemFuncPtr)(void*) = nullptr;

void cltMarriageSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltWeddingHallKindInfo* weddingHallKindInfo,
                                                 cltCoupleRingKindInfo* coupleRingKindInfo,
                                                 void (*chargedRecallQtyByMyItemFuncPtr)(void*)) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclWeddingHallKindInfo = weddingHallKindInfo;
    m_pclCoupleRingKindInfo = coupleRingKindInfo;
    m_pChargedRecallQtyByMyItemFuncPtr = chargedRecallQtyByMyItemFuncPtr;
}

cltMarriageSystem::cltMarriageSystem() = default;
cltMarriageSystem::~cltMarriageSystem() = default;

void cltMarriageSystem::Initialize(void* owner, int now, cltBaseInventory* baseInventory, cltClassSystem* classSystem,
                                   cltMyItemSystem* myItemSystem, CMofMsg* msg, int* outCharged) {
    std::uint8_t married = 0;
    if (outCharged) *outCharged = 0;

    m_pOwner = owner;
    m_pBaseInventory = baseInventory;
    m_pClassSystem = classSystem;
    m_pMyItemSystem = myItemSystem;

    if (msg && (msg->Get_BYTE(&married), married)) {
        m_marriageState = 1;
        msg->Get_Z1(m_spouseName.data(), 0, 0, nullptr);
        msg->Get_DWORD(&m_spouseDBAccount);
        msg->Get_WORD(&m_coupleRingKind);
        msg->Get_DWORD(&m_seekDivorceDBAccount);
        msg->Get_LONG(&m_seekDivorceTime);
        msg->Get_LONG(&m_remainedRecallQty);
        msg->Get_LONG(&m_remainedRecallQtyMyItem);
        msg->Get_LONG(&m_lastChargedRecallQtyTime);
        const int charged = ChargeRecallQty(now);
        if (outCharged) *outCharged = charged;
    } else {
        m_marriageState = 0;
        m_remainedRecallQty = 0;
        m_remainedRecallQtyMyItem = 0;
        m_lastChargedRecallQtyTime = 0;
        m_spouseDBAccount = 0;
        m_coupleRingKind = 0;
        m_seekDivorceDBAccount = 0;
        m_seekDivorceTime = 0;
        std::memset(m_spouseName.data(), 0, m_spouseName.size());
    }
}

void cltMarriageSystem::Free() {
    m_pOwner = nullptr;
    m_pClassSystem = nullptr;
    m_pMyItemSystem = nullptr;
    m_marriageState = 0;
    m_remainedRecallQty = 0;
    m_remainedRecallQtyMyItem = 0;
    m_lastChargedRecallQtyTime = 0;
    m_spouseDBAccount = 0;
    m_coupleRingKind = 0;
    m_seekDivorceDBAccount = 0;
    m_seekDivorceTime = 0;
    std::memset(m_spouseName.data(), 0, m_spouseName.size());
}

std::uint32_t cltMarriageSystem::GetSpouseDBAccount() { return m_spouseDBAccount; }
char* cltMarriageSystem::GetSpouseName() { return m_spouseName.data(); }
std::uint16_t cltMarriageSystem::GetCoupleRingKind() { return m_coupleRingKind; }

void cltMarriageSystem::SetSpouseName(char* spouseName) {
    m_remainedRecallQty = 0;
    m_remainedRecallQtyMyItem = 0;
    m_lastChargedRecallQtyTime = 0;
    if (spouseName) std::strcpy(m_spouseName.data(), spouseName);
}

int cltMarriageSystem::CanChangeCoupleRing(std::uint16_t ringKind) {
    if (IsMarried()) return ringKind != GetCoupleRingKind() ? 0 : 6014;
    return 1;
}

void cltMarriageSystem::ChangeCoupleRing(int now, std::uint16_t ringKind) {
    m_coupleRingKind = ringKind;
    strCoupleRingKindInfo* info = m_pclCoupleRingKindInfo ? m_pclCoupleRingKindInfo->GetCoupleRingKindInfo(ringKind) : nullptr;
    if (!info) return;

    if (info->canSummonSpouse > 0) {
        m_remainedRecallQty = info->canSummonSpouse;
        m_remainedRecallQtyMyItem = cltMyItemSystem::GetSpouseChargeRecallQty(m_pMyItemSystem);
    } else {
        m_remainedRecallQty = 0;
        m_remainedRecallQtyMyItem = 0;
    }
    m_lastChargedRecallQtyTime = now;
}

int cltMarriageSystem::GetMarriageState() { return m_marriageState; }
int cltMarriageSystem::CanPropose() { return !IsMarriageState_None(); }
int cltMarriageSystem::CanBeProposed() { return !IsMarriageState_None(); }
int cltMarriageSystem::CanRecallSpouse() {
    if (IsMarriageState_Married()) return GetRemainedRecallQty_Total() <= 0;
    return 1;
}

void cltMarriageSystem::RecallSpouse() {
    if (m_remainedRecallQtyMyItem >= 1) {
        --m_remainedRecallQtyMyItem;
    } else if (m_remainedRecallQty >= 1) {
        --m_remainedRecallQty;
    }
}

int cltMarriageSystem::GetRemainedRecallQty() { return m_remainedRecallQty; }
int cltMarriageSystem::GetRemainedRecallQty_MyItem() { return m_remainedRecallQtyMyItem; }
int cltMarriageSystem::GetRemainedRecallQty_Total() { return m_remainedRecallQty + m_remainedRecallQtyMyItem; }
int cltMarriageSystem::GetLastChargedRecallQtyTime() { return m_lastChargedRecallQtyTime; }

int cltMarriageSystem::IsMarriageState_None() { return m_marriageState == 0; }
int cltMarriageSystem::IsMarriageState_Married() { return m_marriageState == 1; }

void cltMarriageSystem::Marry(int now, int consumeRing, std::uint32_t spouseDbAccount, char* spouseName,
                              std::uint16_t weddingTicketItemKind, std::uint16_t coupleRingItemKind,
                              std::uint16_t slotA, std::uint16_t slotB, std::uint8_t* outInventoryChanged,
                              cltItemList* outRewardItems, cltItemList* outConsumedItems) {
    cltItemList gained;

    strWeddingHallKindInfo* hall = m_pclWeddingHallKindInfo ? m_pclWeddingHallKindInfo->GetWeddingHallKindInfoByItemKind(weddingTicketItemKind) : nullptr;
    stItemKindInfo* ringItem = m_pclItemKindInfo ? m_pclItemKindInfo->GetItemKindInfo(coupleRingItemKind) : nullptr;
    strCoupleRingKindInfo* ringInfo = (ringItem && m_pclCoupleRingKindInfo)
                                          ? m_pclCoupleRingKindInfo->GetCoupleRingKindInfo(ringItem->u.hunt.wCoupleRingKind)
                                          : nullptr;

    m_spouseDBAccount = spouseDbAccount;
    m_marriageState = 1;
    if (spouseName) std::strcpy(m_spouseName.data(), spouseName);
    m_coupleRingKind = ringInfo ? ringInfo->ringKind : 0;

    if (consumeRing) {
        if (outConsumedItems) {
            if (auto* a = m_pBaseInventory->GetInventoryItem(slotA)) outConsumedItems->AddItem(a->itemKind, 1, 0, 0, 0xFFFF, nullptr);
            if (auto* b = m_pBaseInventory->GetInventoryItem(slotB)) outConsumedItems->AddItem(b->itemKind, 1, 0, 0, 0xFFFF, nullptr);
        }
        m_pBaseInventory->DelInventoryItem(slotA, 1, outInventoryChanged);
        m_pBaseInventory->DelInventoryItem(slotB, 1, outInventoryChanged);
    }

    if (hall) {
        const std::uint16_t* rewardPair = &hall->bouquetItemKind;
        for (int i = 0; i < 1; ++i) {
            if (!rewardPair[0] || !rewardPair[1]) break;
            gained.AddItem(rewardPair[0], rewardPair[1], 0, 0, 0xFFFF, nullptr);
            if (outRewardItems) outRewardItems->AddItem(rewardPair[0], rewardPair[1], 0, 0, 0xFFFF, nullptr);
            rewardPair += 2;
        }
    }

    if (gained.GetItemsNum()) {
        m_pBaseInventory->AddInventoryItem(&gained, outInventoryChanged);
    }

    ChargeRecallQty(now);
}

int cltMarriageSystem::IsMarried() { return m_marriageState == 1; }

int cltMarriageSystem::CanSeekDivorce(std::uint32_t dbAccount) {
    int result = IsMarried();
    if (result) result = !IsSeekDivorce() || m_seekDivorceDBAccount != dbAccount;
    return result;
}

int cltMarriageSystem::SeekDivorce(std::uint32_t dbAccount, int time) {
    if (m_seekDivorceDBAccount) {
        OnDivorced();
        return 1;
    }
    m_seekDivorceDBAccount = dbAccount;
    m_seekDivorceTime = time;
    return 0;
}

int cltMarriageSystem::OnSeekDivorced(std::uint32_t dbAccount, int time) {
    if (m_seekDivorceDBAccount) {
        OnDivorced();
        return 1;
    }
    m_seekDivorceDBAccount = dbAccount;
    m_seekDivorceTime = time;
    return 0;
}

int cltMarriageSystem::IsSeekDivorce() { return m_seekDivorceDBAccount != 0; }
std::uint32_t cltMarriageSystem::GetSeekDivorceDBAccount() { return m_seekDivorceDBAccount; }
int cltMarriageSystem::GetSeekDivorceTime() { return m_seekDivorceTime; }
int cltMarriageSystem::DidISeekDivorce() {
    int result = IsSeekDivorce();
    if (result) result = m_seekDivorceDBAccount != m_spouseDBAccount;
    return result;
}

int cltMarriageSystem::CanCancelSeekDivorce(std::uint32_t dbAccount) {
    int result = IsSeekDivorce();
    if (result) result = m_seekDivorceDBAccount == dbAccount;
    return result;
}

void cltMarriageSystem::CancelSeekDivorce() {
    m_seekDivorceDBAccount = 0;
    m_seekDivorceTime = 0;
}

void cltMarriageSystem::OnCanceledSeekDivorce() {
    m_seekDivorceDBAccount = 0;
    m_seekDivorceTime = 0;
}

void cltMarriageSystem::OnDivorced() {
    m_marriageState = 0;
    m_spouseDBAccount = 0;
    m_remainedRecallQty = 0;
    m_remainedRecallQtyMyItem = 0;
    m_lastChargedRecallQtyTime = 0;
    m_seekDivorceDBAccount = 0;
    m_seekDivorceTime = 0;
    std::memset(m_spouseName.data(), 0, m_spouseName.size());
}

int cltMarriageSystem::GetCoupleRingExpAdvantage() {
    auto* info = m_pclCoupleRingKindInfo ? m_pclCoupleRingKindInfo->GetCoupleRingKindInfo(m_coupleRingKind) : nullptr;
    return info ? info->expRatePercent : 0;
}

int cltMarriageSystem::ChargeRecallQty(int now) {
    std::tm current = *std::localtime(reinterpret_cast<const std::time_t*>(&now));
    std::time_t lastTs = static_cast<std::time_t>(m_lastChargedRecallQtyTime);
    std::tm last = *std::localtime(&lastTs);

    if (current.tm_year == last.tm_year && current.tm_yday == last.tm_yday) return 0;

    const std::uint16_t ringKind = m_coupleRingKind;
    m_remainedRecallQty = 0;
    m_remainedRecallQtyMyItem = 0;

    auto* info = m_pclCoupleRingKindInfo ? m_pclCoupleRingKindInfo->GetCoupleRingKindInfo(ringKind) : nullptr;
    if (!info) return 0;

    m_remainedRecallQty = info->canSummonSpouse;
    m_remainedRecallQtyMyItem = cltMyItemSystem::GetSpouseChargeRecallQty(m_pMyItemSystem);
    m_lastChargedRecallQtyTime = now;
    return 1;
}

void cltMarriageSystem::OnChargeRecallQtyByMyItem(int chargedQty) {
    auto* info = m_pclCoupleRingKindInfo ? m_pclCoupleRingKindInfo->GetCoupleRingKindInfo(m_coupleRingKind) : nullptr;
    if (info && info->canSummonSpouse > 0) {
        m_remainedRecallQtyMyItem = chargedQty;
        if (m_pChargedRecallQtyByMyItemFuncPtr) {
            m_pChargedRecallQtyByMyItemFuncPtr(m_pOwner);
        }
    }
}
