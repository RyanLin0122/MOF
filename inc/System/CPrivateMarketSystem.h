#pragma once

#include <cstdint>
#include <cstring>

class cltBaseInventory;
class cltItemKindInfo;

#pragma pack(push, 1)
struct stSellingItem {
    std::uint16_t itemKind = 0;
    std::uint16_t invenSlot = 0;
    std::uint16_t qty = 0;
    std::uint16_t _pad = 0;
    std::uint32_t price = 0;
    std::uint32_t param = 0;
};
#pragma pack(pop)
static_assert(sizeof(stSellingItem) == 16, "stSellingItem size must be 16 bytes");

class CPrivateMarketSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* pItemKindInfo);

    CPrivateMarketSystem();
    ~CPrivateMarketSystem();

    int Initialize(cltBaseInventory* pInventory);
    int Initialize();

    int CanAddPrivateMarketList(std::uint16_t slot, std::uint16_t itemKind, std::uint16_t invenSlot, std::uint16_t qty, unsigned int price);
    int AddPrivateMarketList(std::uint16_t slot, std::uint16_t itemKind, std::uint16_t invenSlot, std::uint16_t qty, unsigned int price, unsigned int param);
    int AddItem(std::uint16_t slot, std::uint16_t itemKind, std::uint16_t invenSlot, std::uint16_t qty, unsigned int price, unsigned int param);
    int SetItemQty(std::uint16_t slot, std::uint16_t qty);

    int CanDelPrivateMarketList(std::uint16_t slot);
    std::uint16_t DelPrivateMarketList(std::uint16_t slot);
    std::uint16_t DelItem(std::uint16_t slot);

    void PrivateMarketItemToInventory();

    std::uint16_t CanSellPrivateMarketItem(std::uint16_t slot, std::uint16_t qty);
    void SellPrivateMarketItem(std::uint16_t slot, std::uint16_t qty);

    int IsEmptySlot(std::uint16_t slot);
    std::int16_t GetEmptySlot();

    int CanGetSellingItemList();
    stSellingItem* GetSellingItemList();
    stSellingItem* GetSellingItemInfo(std::uint16_t slot);

    int IncreaseSellingItemListCount();
    int DecreaseSellingItemListCount();

    int IsExistInvenSlot(std::uint16_t invenSlot);

    static constexpr int MAX_SELLING_ITEMS = 8;

private:
    stSellingItem m_sellingItems[MAX_SELLING_ITEMS];
    cltBaseInventory* m_pInventory = nullptr;
    std::uint16_t m_pad0 = 0;
    std::uint16_t m_sellingCount = 0;

    static cltItemKindInfo* m_pclItemKindInfo;
};

extern CPrivateMarketSystem g_clPrivateMarketSystem;
