#include "System/cltStorageSystem.h"

#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "Network/CMofMsg.h"
#include "System/cltMoneySystem.h"
#include "System/cltQuestSystem.h"

cltItemKindInfo* cltStorageSystem::m_pclItemKindInfo = nullptr;

// 反編譯對應：cltStorageSystem::InitializeStaticVariable
// 原始碼 mofclient.c:336259
void cltStorageSystem::InitializeStaticVariable(cltItemKindInfo* pItemKindInfo) {
    m_pclItemKindInfo = pItemKindInfo;
}

// 反編譯對應：cltStorageSystem::cltStorageSystem
// 原始碼 mofclient.c:336265
// GT: 先逐項清零 items，再清 depositMoney、bagNum、m_pInventory、m_pMoney
// 注意：GT 不清 m_pQuest（offset +8），保留未初始化狀態
cltStorageSystem::cltStorageSystem() {
    std::memset(m_items, 0, sizeof(m_items));
    m_depositMoney = 0;
    m_bagNum = 0;
    m_pInventory = nullptr;
    m_pMoney = nullptr;
}

// 反編譯對應：cltStorageSystem::Initialize (cltItemList 版)
// 原始碼 mofclient.c:336291
void cltStorageSystem::Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, cltQuestSystem* pQuest,
                                  int depositMoney, char bagNum, cltItemList* pItemList) {
    m_pMoney = pMoney;
    m_pInventory = pInventory;
    m_bagNum = static_cast<std::uint8_t>(bagNum);
    m_pQuest = pQuest;
    m_depositMoney = depositMoney;

    std::memset(m_items, 0, sizeof(m_items));

    if (pItemList) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQty = 0;
        std::uint16_t itemPos = 0;
        unsigned int itemValue = 0;

        std::uint16_t itemCount = pItemList->GetItemsNum();
        for (int i = 0; i < itemCount; ++i) {
            pItemList->GetItem(i, &itemKind, &itemQty, &itemValue, &itemPos, nullptr);
            m_items[itemPos].itemKind = itemKind;
            m_items[itemPos].itemQty = itemQty;
            m_items[itemPos].value0 = itemValue;
        }
    }
}

// 反編譯對應：cltStorageSystem::Initialize (CMofMsg 版)
// 原始碼 mofclient.c:336332
void cltStorageSystem::Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, cltQuestSystem* pQuest,
                                  CMofMsg* pMsg) {
    m_pInventory = pInventory;
    m_pMoney = pMoney;
    m_pQuest = pQuest;

    pMsg->Get_BYTE(&m_bagNum);
    pMsg->Get_LONG(&m_depositMoney);

    std::uint16_t itemCount = 0;
    pMsg->Get_WORD(&itemCount);

    if (pMsg) {
        for (int i = 0; i < itemCount; ++i) {
            std::uint8_t slot = 0;
            std::uint16_t kind = 0;
            std::uint16_t qty = 0;
            std::uint8_t extra = 0;
            pMsg->Get_BYTE(&slot);
            pMsg->Get_WORD(&kind);
            pMsg->Get_WORD(&qty);
            pMsg->Get_BYTE(&extra);
            m_items[slot].itemKind = kind;
            m_items[slot].itemQty = qty;
            m_items[slot].value1 = static_cast<std::uint32_t>(extra);
        }

        std::uint16_t valueCount = 0;
        pMsg->Get_WORD(&valueCount);
        for (int i = 0; i < valueCount; ++i) {
            std::uint8_t slot = 0;
            unsigned int value = 0;
            pMsg->Get_BYTE(&slot);
            pMsg->Get_DWORD(&value);
            m_items[slot].value0 = value;
        }
    }
}

// 反編譯對應：cltStorageSystem::Free
// 原始碼 mofclient.c:336394
void cltStorageSystem::Free() {
    m_bagNum = 0;
    m_depositMoney = 0;
    m_pInventory = nullptr;
    m_pMoney = nullptr;
    std::memset(m_items, 0, sizeof(m_items));
}

// 反編譯對應：cltStorageSystem::GetEmptySlotNum
// 原始碼 mofclient.c:336404
int cltStorageSystem::GetEmptySlotNum(int startIdx, int endIdx) {
    int count = 0;
    for (int i = startIdx; i <= endIdx; ++i) {
        if (!m_items[i].itemKind)
            ++count;
    }
    return count;
}

// 反編譯對應：cltStorageSystem::GetMaxAddAbleItemQty
// 原始碼 mofclient.c:336428
// 取得可加入的最大物品數量，根據空格數 * 最大堆疊 + 現有同物品可追加量
int cltStorageSystem::GetMaxAddAbleItemQty(std::uint16_t itemKind) {
    stItemKindInfo* info = m_pclItemKindInfo->GetItemKindInfo(itemKind);
    if (!info)
        return 0;

    int startIdx, endIdx;
    if (m_pclItemKindInfo->IsFashionItem(itemKind)) {
        startIdx = FASHION_SLOT_MIN;
        endIdx = FASHION_SLOT_MAX;
    } else {
        startIdx = 0;
        endIdx = SLOTS_PER_BAG * m_bagNum - 1;
        if (endIdx > NORMAL_SLOT_MAX)
            endIdx = NORMAL_SLOT_MAX;
    }

    int pileUp = info->m_byMaxPileUpNum;
    int result = GetEmptySlotNum(startIdx, endIdx) * pileUp;

    for (int i = startIdx; i <= endIdx; ++i) {
        if (m_items[i].itemKind == itemKind)
            result += pileUp - m_items[i].itemQty;
    }
    return result;
}

// 反編譯對應：cltStorageSystem::CanAddItemByItemKind
// 原始碼 mofclient.c:336472
int cltStorageSystem::CanAddItemByItemKind(std::uint16_t itemKind, std::uint16_t qty) {
    int maxQty = GetMaxAddAbleItemQty(itemKind);
    if (!maxQty)
        return 0;
    return maxQty >= qty;
}

// 反編譯對應：cltStorageSystem::CanAddItem
// 原始碼 mofclient.c:336483
// 回傳值作為 boolean 使用（0 或非零）
strInventoryItem* cltStorageSystem::CanAddItem(std::uint8_t invenSlot, std::uint16_t qty) {
    if (!qty)
        return nullptr;
    if (!m_pInventory->CanDelInventoryItem(invenSlot, qty))
        return nullptr;

    auto* invItem = m_pInventory->GetInventoryItem(invenSlot);
    if (!invItem)
        return nullptr;

    if (invItem->itemQty < qty)
        return nullptr;

    if (!m_pclItemKindInfo->IsStorageItem(invItem->itemKind))
        return nullptr;

    return reinterpret_cast<strInventoryItem*>(CanAddItemByItemKind(invItem->itemKind, qty) != 0 ? 1 : 0);
}

// 反編譯對應：cltStorageSystem::AddItem
// 原始碼 mofclient.c:336513
void cltStorageSystem::AddItem(char invenSlot, std::uint16_t qty, std::uint8_t* changeSlots,
                               std::uint16_t* questKinds, unsigned int* questValues) {
    int remaining = qty;
    auto* invItem = m_pInventory->GetInventoryItem(static_cast<std::uint8_t>(invenSlot));
    stItemKindInfo* kindInfo = m_pclItemKindInfo->GetItemKindInfo(invItem->itemKind);

    int startIdx, endIdx;
    if (m_pclItemKindInfo->IsFashionItem(invItem->itemKind)) {
        startIdx = FASHION_SLOT_MIN;
        endIdx = FASHION_SLOT_MAX;
    } else {
        startIdx = 0;
        endIdx = SLOTS_PER_BAG * m_bagNum - 1;
        if (endIdx > NORMAL_SLOT_MAX)
            endIdx = NORMAL_SLOT_MAX;
    }

    int pileUp = kindInfo->m_byMaxPileUpNum;
    int origStart = startIdx;

    // 先填入現有同物品的堆疊
    for (int i = startIdx; i <= endIdx && remaining > 0; ++i) {
        if (m_items[i].itemKind == invItem->itemKind) {
            if (m_items[i].itemQty < pileUp) {
                int canAdd = pileUp - m_items[i].itemQty;
                if (canAdd > remaining)
                    canAdd = remaining;
                remaining -= canAdd;
                m_items[i].itemQty += canAdd;
                if (changeSlots)
                    changeSlots[i] = 1;
                if (!remaining)
                    goto done;
            }
        }
    }

    // 再填入空格
    while (remaining > 0) {
        int emptyIdx = GetEmptySlotIndex(origStart, endIdx);
        if (emptyIdx == -1)
            break;
        m_items[emptyIdx].itemKind = invItem->itemKind;
        m_items[emptyIdx].value0 = invItem->value0;
        m_items[emptyIdx].value1 = invItem->value1;
        int toAdd = pileUp;
        if (toAdd > remaining)
            toAdd = remaining;
        m_items[emptyIdx].itemQty = toAdd;
        remaining -= toAdd;
        if (changeSlots)
            changeSlots[emptyIdx] = 1;
    }

done:
    m_pInventory->DelInventoryItem(static_cast<std::uint8_t>(invenSlot), qty, nullptr);
    m_pQuest->CompleteFunctionQuest(10, questKinds, questValues);
}

// 反編譯對應：cltStorageSystem::CanDelItem
// 原始碼 mofclient.c:336601
int cltStorageSystem::CanDelItem(std::uint8_t slot, std::uint16_t qty) {
    if (!qty)
        return 0;

    std::uint16_t kind = 0;
    std::uint16_t storedQty = 0;
    unsigned int value = 0;

    if (!GetStorageItem(slot, &kind, &storedQty, &value))
        return 0;

    if (!kind)
        return 0;

    if (storedQty < qty)
        return 0;

    return m_pInventory->CanAddInventoryItem(kind, qty) == 0;
}

// 反編譯對應：cltStorageSystem::DelItem
// 原始碼 mofclient.c:336632
void cltStorageSystem::DelItem(std::uint8_t slot, std::uint16_t qty, std::uint8_t* changeSlots) {
    m_items[slot].itemQty -= qty;

    // 建構 strInventoryItem 傳給背包
    strInventoryItem newItem;
    newItem.itemKind = m_items[slot].itemKind;
    newItem.itemQty = qty;
    newItem.value0 = m_items[slot].value0;
    newItem.value1 = m_items[slot].value1;

    m_pInventory->AddInventoryItem(&newItem, changeSlots, nullptr);

    if (!m_items[slot].itemQty) {
        m_items[slot].itemKind = 0;
        m_items[slot].value0 = 0;
        m_items[slot].value1 = 0;
    }
}

// 反編譯對應：cltStorageSystem::CanMoveItem
// 原始碼 mofclient.c:336661
int cltStorageSystem::CanMoveItem(std::uint8_t from, std::uint8_t to) {
    auto* fromItem = GetStorageItem(from);
    if (!fromItem)
        return 0;
    if (!GetStorageItem(to))
        return 0;

    std::uint16_t kind = fromItem->itemKind;
    if (!kind || !m_pclItemKindInfo->GetItemKindInfo(kind))
        return 0;

    // from 在一般倉庫：to 也必須在一般倉庫
    // from 在時裝倉庫：to 也必須在時裝倉庫
    if (from > NORMAL_SLOT_MAX) {
        if (from < FASHION_SLOT_MIN || from > FASHION_SLOT_MAX ||
            to < FASHION_SLOT_MIN || to > FASHION_SLOT_MAX)
            return 0;
    } else {
        if (to > NORMAL_SLOT_MAX)
            return 0;
    }
    return 1;
}

// 反編譯對應：cltStorageSystem::MoveItem
// 原始碼 mofclient.c:336687
void cltStorageSystem::MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* changeSlots) {
    auto* fromItem = GetStorageItem(from);
    auto* toItem = GetStorageItem(to);

    if (toItem->itemKind) {
        if (fromItem->itemKind == toItem->itemKind) {
            // 堆疊合併
            stItemKindInfo* info = m_pclItemKindInfo->GetItemKindInfo(fromItem->itemKind);
            if (info) {
                int canMove = info->m_byMaxPileUpNum - toItem->itemQty;
                std::uint16_t moveQty = fromItem->itemQty;
                if (canMove < moveQty)
                    moveQty = canMove;
                toItem->itemQty += moveQty;
                fromItem->itemQty -= moveQty;
                if (!fromItem->itemQty) {
                    fromItem->itemKind = 0;
                    fromItem->value0 = 0;
                    fromItem->value1 = 0;
                }
            }
        } else {
            // 交換
            strStorageItem temp = *fromItem;
            *fromItem = *toItem;
            *toItem = temp;
        }
    } else {
        // 移動到空格
        *toItem = *fromItem;
        fromItem->itemKind = 0;
        fromItem->itemQty = 0;
        fromItem->value0 = 0;
        fromItem->value1 = 0;
    }

    if (changeSlots) {
        changeSlots[from] = 1;
        changeSlots[to] = 1;
    }
}

// 反編譯對應：cltStorageSystem::CanDepositMoney
// 原始碼 mofclient.c:336757
int cltStorageSystem::CanDepositMoney(int amount) {
    if (!m_pMoney->CanDecreaseMoney(amount))
        return 0;
    return amount + m_depositMoney <= MAX_DEPOSIT;
}

// 反編譯對應：cltStorageSystem::DepositMoney
// 原始碼 mofclient.c:336768
void cltStorageSystem::DepositMoney(int amount) {
    m_depositMoney += amount;
    m_pMoney->DecreaseMoney(amount);
}

// 反編譯對應：cltStorageSystem::CanWithdrawingMoney
// 原始碼 mofclient.c:336775
int cltStorageSystem::CanWithdrawingMoney(int amount) {
    if (amount <= 0)
        return 0;
    if (m_depositMoney < amount)
        return 0;
    return m_pMoney->CanIncreaseMoney(amount);
}

// 反編譯對應：cltStorageSystem::WithdrawingMoney
// 原始碼 mofclient.c:336785
void cltStorageSystem::WithdrawingMoney(int amount) {
    m_depositMoney -= amount;
    m_pMoney->IncreaseMoney(amount);
}

// 反編譯對應：cltStorageSystem::GetStorageItem (指標版)
// 原始碼 mofclient.c:336792
strStorageItem* cltStorageSystem::GetStorageItem(std::uint8_t slot) {
    if (IsValidSlot(slot))
        return &m_items[slot];
    return nullptr;
}

// 反編譯對應：cltStorageSystem::GetStorageItem (輸出版)
// 原始碼 mofclient.c:336804
int cltStorageSystem::GetStorageItem(std::uint8_t slot, std::uint16_t* outKind, std::uint16_t* outQty, unsigned int* outValue) {
    if (!IsValidSlot(slot))
        return 0;
    *outKind = m_items[slot].itemKind;
    *outQty = m_items[slot].itemQty;
    *outValue = m_items[slot].value0;
    return 1;
}

// 反編譯對應：cltStorageSystem::GetDepositMoney
// 原始碼 mofclient.c:336818
int cltStorageSystem::GetDepositMoney() {
    return m_depositMoney;
}

// 反編譯對應：cltStorageSystem::GetMaxDepositAbleMoney
// 原始碼 mofclient.c:336824
int cltStorageSystem::GetMaxDepositAbleMoney() {
    int maxDepositable = MAX_DEPOSIT - m_depositMoney;
    int currentMoney = m_pMoney->GetGameMoney();
    return maxDepositable < currentMoney ? maxDepositable : currentMoney;
}

// 反編譯對應：cltStorageSystem::CanExpandGenericBag
// 原始碼 mofclient.c:336838
int cltStorageSystem::CanExpandGenericBag() {
    if (m_bagNum >= MAX_BAG_NUM)
        return 700;
    int cost = GetMoneyToExpandGenericBag();
    return m_pMoney->CanDecreaseMoney(cost) ? 0 : 701;
}

// 反編譯對應：cltStorageSystem::ExpandGenericBag
// 原始碼 mofclient.c:336849
int cltStorageSystem::ExpandGenericBag() {
    int cost = GetMoneyToExpandGenericBag();
    m_pMoney->DecreaseMoney(cost);
    ++m_bagNum;
    return cost;
}

// 反編譯對應：cltStorageSystem::GetMoneyToExpandGenericBag
// 原始碼 mofclient.c:336860
int cltStorageSystem::GetMoneyToExpandGenericBag() {
    if (m_bagNum == 1)
        return 10000;
    if (m_bagNum == 2)
        return 100000;
    return 0;
}

// 反編譯對應：cltStorageSystem::GetGenericBagNum
// 原始碼 mofclient.c:336870
std::uint8_t cltStorageSystem::GetGenericBagNum() {
    return m_bagNum;
}

// 反編譯對應：cltStorageSystem::GetEmptySlotIndex
// 原始碼 mofclient.c:336876
int cltStorageSystem::GetEmptySlotIndex(int startIdx, int endIdx) {
    if (startIdx > endIdx)
        return -1;
    for (int i = startIdx; i <= endIdx; ++i) {
        if (!m_items[i].itemKind)
            return i;
    }
    return -1;
}

// 反編譯對應：cltStorageSystem::GetInventorySystem
// 原始碼 mofclient.c:336893
cltBaseInventory* cltStorageSystem::GetInventorySystem() {
    return m_pInventory;
}

// 反編譯對應：cltStorageSystem::GetMoneySystem
// 原始碼 mofclient.c:336899
cltMoneySystem* cltStorageSystem::GetMoneySystem() {
    return m_pMoney;
}

// 反編譯對應：cltStorageSystem::IsExistItem
// 原始碼 mofclient.c:336905
int cltStorageSystem::IsExistItem(std::uint16_t itemKind) {
    if (!itemKind)
        return 0;
    for (int i = 0; i < MAX_STORAGE_SLOTS; ++i) {
        if (m_items[i].itemKind == itemKind)
            return 1;
    }
    return 0;
}

// 反編譯對應：cltStorageSystem::IsExistMoveServer
// 原始碼 mofclient.c:336922
// 檢查倉庫中是否有可轉服的物品（m_bUseChangeServer）
int cltStorageSystem::IsExistMoveServer() {
    for (int i = 0; i < MAX_STORAGE_SLOTS; ++i) {
        if (m_items[i].itemKind) {
            stItemKindInfo* info = m_pclItemKindInfo->GetItemKindInfo(m_items[i].itemKind);
            if (info && info->Instant.m_bUseChangeServer)
                return 1;
        }
    }
    return 0;
}

// 反編譯對應：cltStorageSystem::SetItemSealed
// 原始碼 mofclient.c:336947
// 設定指定 slot 的 value1（封印值）
int cltStorageSystem::SetItemSealed(std::uint16_t slot, int sealValue, int /*unused*/) {
    m_items[slot].value1 = static_cast<std::uint32_t>(sealValue);
    return 3 * (slot + 2);
}

// 私有輔助：驗證 slot 是否在有效範圍
bool cltStorageSystem::IsValidSlot(std::uint8_t slot) const {
    return slot <= NORMAL_SLOT_MAX || (slot >= FASHION_SLOT_MIN && slot <= FASHION_SLOT_MAX);
}
