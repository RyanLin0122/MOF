#pragma once

#include <cstdint>
#include <cstring>

class cltBaseInventory;
class cltMoneySystem;
class cltItemKindInfo;
class cltItemList;
class CMofMsg;
struct stItemKindInfo;

#pragma pack(push, 1)
struct strExStorageItem {
    std::uint16_t itemKind = 0;
    std::uint16_t itemQty = 0;
    std::uint32_t value = 0;
};
#pragma pack(pop)
static_assert(sizeof(strExStorageItem) == 8, "strExStorageItem size must be 8 bytes");

class cltExStorageSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* pItemKindInfo);

    cltExStorageSystem();
    ~cltExStorageSystem();

    void Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, int depositMoney, char bagNum, cltItemList* pItemList);
    void Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, CMofMsg* pMsg);
    void Free();

    int GetEmptySlotNum();
    int GetMaxAddAbleItemQty(std::uint16_t itemKind);
    int CanAddItemByItemKind(std::uint16_t itemKind, std::uint16_t qty);
    strExStorageItem* CanAddItem(std::uint8_t invenSlot, std::uint16_t qty);
    void AddItem(char invenSlot, std::uint16_t qty, std::uint8_t* changeSlots);

    int CanDelItem(std::uint8_t slot, std::uint16_t qty);
    void DelItem(std::uint8_t slot, std::uint16_t qty, std::uint8_t* changeSlots);

    stItemKindInfo* CanMoveItem(std::uint8_t from, std::uint8_t to);
    void MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* changeSlots);

    int CanDepositMoney(int amount);
    int DepositMoney(int amount);
    int CanWithdrawingMoney(int amount);
    void WithdrawingMoney(int amount);

    int GetStorageItem(std::uint8_t slot, std::uint16_t* outKind, std::uint16_t* outQty, unsigned int* outValue);
    strExStorageItem* GetStorageItem(std::uint8_t slot);

    int GetDepositMoney();
    int GetMaxDepositAbleMoney();

    unsigned int CanExpandGenericBag(int expandType);
    int ExpandGenericBag(int expandType);
    int GetMoneyToExpandGenericBag();
    std::uint8_t GetGenericBagNum();
    int GetEmptySlotIndex();

    cltBaseInventory* GetInventorySystem();
    cltMoneySystem* GetMoneySystem();

    std::int64_t GetChargeMoney(int amount);
    int GetDepostitChargeRate();

    static constexpr int MAX_STORAGE_SLOTS = 255;
    static constexpr int SLOTS_PER_BAG = 24;
    static constexpr int MAX_BAG_NUM = 4;
    static constexpr int MAX_DEPOSIT = 2000000000;

private:
    cltBaseInventory* m_pInventory = nullptr;
    cltMoneySystem* m_pMoney = nullptr;
    std::uint8_t m_genericBagNum = 0;
    std::uint8_t m_padding[3]{};
    strExStorageItem m_items[MAX_STORAGE_SLOTS]{};
    int m_depositMoney = 0;

    static cltItemKindInfo* m_pclItemKindInfo;
};

extern cltExStorageSystem g_clExStorageSystem;
