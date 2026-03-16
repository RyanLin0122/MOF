#pragma once

#include <cstdint>
#include <cstring>

class cltBaseInventory;
class cltMoneySystem;
class cltQuestSystem;
class cltItemKindInfo;
class cltItemList;
class CMofMsg;
struct stItemKindInfo;
struct strInventoryItem;

// 反編譯對應：倉庫系統物品結構
// 每個物品 12 bytes（3 DWORDs）
#pragma pack(push, 1)
struct strStorageItem {
    std::uint16_t itemKind = 0;
    std::uint16_t itemQty = 0;
    std::uint32_t value0 = 0;
    std::uint32_t value1 = 0;
};
#pragma pack(pop)
static_assert(sizeof(strStorageItem) == 12, "strStorageItem size must be 12 bytes");

// 反編譯對應：cltStorageSystem
// 記憶體佈局：
//   offset 0:    cltBaseInventory*
//   offset 4:    cltMoneySystem*
//   offset 8:    cltQuestSystem*
//   offset 12:   uint8_t bagNum
//   offset 16:   strStorageItem[255] (3060 bytes)
//   offset 3076: int depositMoney
class cltStorageSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* pItemKindInfo);

    cltStorageSystem();
    ~cltStorageSystem() = default;

    void Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, cltQuestSystem* pQuest,
                    int depositMoney, char bagNum, cltItemList* pItemList);
    void Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, cltQuestSystem* pQuest,
                    CMofMsg* pMsg);
    void Free();

    int GetEmptySlotNum(int startIdx, int endIdx);
    int GetMaxAddAbleItemQty(std::uint16_t itemKind);
    int CanAddItemByItemKind(std::uint16_t itemKind, std::uint16_t qty);
    strInventoryItem* CanAddItem(std::uint8_t invenSlot, std::uint16_t qty);
    void AddItem(char invenSlot, std::uint16_t qty, std::uint8_t* changeSlots,
                 std::uint16_t* questKinds, unsigned int* questValues);

    int CanDelItem(std::uint8_t slot, std::uint16_t qty);
    void DelItem(std::uint8_t slot, std::uint16_t qty, std::uint8_t* changeSlots);

    int CanMoveItem(std::uint8_t from, std::uint8_t to);
    void MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* changeSlots);

    int CanDepositMoney(int amount);
    void DepositMoney(int amount);
    int CanWithdrawingMoney(int amount);
    void WithdrawingMoney(int amount);

    strStorageItem* GetStorageItem(std::uint8_t slot);
    int GetStorageItem(std::uint8_t slot, std::uint16_t* outKind, std::uint16_t* outQty, unsigned int* outValue);

    int GetDepositMoney();
    int GetMaxDepositAbleMoney();

    int CanExpandGenericBag();
    int ExpandGenericBag();
    int GetMoneyToExpandGenericBag();
    std::uint8_t GetGenericBagNum();

    int GetEmptySlotIndex(int startIdx, int endIdx);

    cltBaseInventory* GetInventorySystem();
    cltMoneySystem* GetMoneySystem();

    int IsExistItem(std::uint16_t itemKind);
    int IsExistMoveServer();
    int SetItemSealed(std::uint16_t slot, int sealValue, int unused);

    // 有效的倉庫 slot 範圍
    static constexpr int NORMAL_SLOT_MAX = 0x47;      // 71
    static constexpr int FASHION_SLOT_MIN = 0xA8;     // 168
    static constexpr int FASHION_SLOT_MAX = 0xBF;     // 191
    static constexpr int MAX_STORAGE_SLOTS = 255;
    static constexpr int SLOTS_PER_BAG = 24;
    static constexpr int MAX_BAG_NUM = 3;
    static constexpr int MAX_DEPOSIT = 2000000000;

private:
    bool IsValidSlot(std::uint8_t slot) const;

    cltBaseInventory* m_pInventory = nullptr;
    cltMoneySystem* m_pMoney = nullptr;
    cltQuestSystem* m_pQuest = nullptr;
    std::uint8_t m_bagNum = 0;
    std::uint8_t m_padding[3]{};
    strStorageItem m_items[MAX_STORAGE_SLOTS]{};
    int m_depositMoney = 0;

    static cltItemKindInfo* m_pclItemKindInfo;
};
