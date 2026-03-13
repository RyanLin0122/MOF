#pragma once

#include <array>
#include <cstdint>
#include <algorithm>
#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Network/CMofMsg.h"

struct strPetInventoryItem {
    std::uint16_t itemKind = 0;
    std::uint16_t itemQty = 0;
};

class cltPetInventorySystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo);

    cltPetInventorySystem();

    void Initialize(CMofMsg* msg);
    void Free();

    int CanAddItem(std::uint16_t itemKind, std::uint16_t itemQty);
    void AddItem(std::uint16_t itemKind, int itemQty, std::uint8_t* outChangedSlots);
    void AddItem(std::uint8_t slot, std::uint16_t itemKind, std::uint16_t itemQty, std::uint8_t* outChangedSlots);

    int CanMoveItem(std::uint8_t from, std::uint8_t to);
    void MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* outChangedSlots);

    int CanDelItemBySlot(std::uint8_t slot, std::uint16_t qty);
    int CanDelItemBySlot(std::uint8_t slot);
    int CanDelItemByItemKind(std::uint16_t itemKind, std::uint16_t qty);

    void DelItemBySlot(std::uint8_t slot, std::uint16_t qty, std::uint8_t* outChangedSlots);
    void DelItemBySlot(std::uint8_t slot, std::uint8_t* outChangedSlots);
    void DelItemByItemKind(std::uint16_t itemKind, std::uint16_t qty, std::uint8_t* outChangedSlots);

    int IsValidSlot(std::uint8_t slot);
    strPetInventoryItem* GetPetInventoryItem(std::uint8_t slot);

    std::uint8_t GetPetBagNum();
    void EmptyPetInventoryItem(std::uint8_t slot);

    int CanIncreasePetBagNum();
    void IncreasePetBagNum();

    void OnPetCreated(std::uint8_t bagInitCount);
    void OnPetDeleted();

    int GetEmptyItemSlotNum();
    int GetFirstEmptyItemSlot();
    int IsPetInventoryEmpty();
    std::uint16_t GetInventoryItemQuantity(std::uint16_t itemKind);
    int GetSlotNumByItemID(std::uint16_t itemKind);

    static cltItemKindInfo* m_pclItemKindInfo;

private:
    static constexpr int kMaxSlots = 255;
    static constexpr int kMaxBags = 5;
    static constexpr int kSlotsPerBag = 12;

    std::array<strPetInventoryItem, kMaxSlots> m_items{};
    std::uint8_t m_bagNum = 0;
};
