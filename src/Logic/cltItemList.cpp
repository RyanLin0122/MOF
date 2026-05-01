#include "Logic/cltItemList.h"

#include <cstdio>
#include <cstring>
#include <cstdint>

#include "Network/CMofMsg.h"
#include "Logic/cltBaseInventory.h"
#include "Info/cltShopInfo.h"
#include "Info/cltItemKindInfo.h"

cltShopInfo* cltItemList::m_pclShopInfo = nullptr;
cltItemKindInfo* cltItemList::m_pclItemKindInfo = nullptr;

void cltItemList::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltShopInfo* shopInfo) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclShopInfo = shopInfo;
}

cltItemList::cltItemList() {
    SetCaps(3u);
    Initialize();
    SetBaseInventory(nullptr);
}

cltItemList::~cltItemList() = default;

void cltItemList::Initialize() {
    m_itemsNum = 0;
    std::memset(&m_entries[0], 0, 0x2710u);
}

void cltItemList::Initialize(unsigned int a2) {
    Initialize();
    SetCaps(a2);
}

void cltItemList::Initialize(unsigned int caps, CMofMsg* message, cltBaseInventory* baseInventory) {
    std::uint16_t totalItemCount = 0;

    SetCaps(caps);
    SetBaseInventory(baseInventory);
    message->Get_WORD(&totalItemCount);

    for (int itemIndex = 0; itemIndex < totalItemCount; ++itemIndex) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQuantity = 0;
        std::uint16_t itemPosition = 0;

        if ((GetCaps() & 1) != 0) {
            message->Get_WORD(&itemKind);
        }
        if ((GetCaps() & 2) != 0) {
            message->Get_WORD(&itemQuantity);
        }
        if ((GetCaps() & 4) != 0) {
            message->Get_WORD(&itemPosition);
        }

        AddItem(
            static_cast<std::int16_t>(itemKind),
            static_cast<std::int16_t>(itemQuantity),
            0,
            0,
            itemPosition,
            nullptr);
    }
}

int cltItemList::AddItem(
    std::int16_t itemKind,
    std::int16_t itemQuantity,
    int value0,
    int sealedStatus,
    std::uint16_t itemPosition,
    std::uint32_t* outIndex) {
    if (m_itemsNum >= 0x1F4u) {
        return 0;
    }

    if ((GetCaps() & 1) != 0) {
        m_entries[m_itemsNum].itemKind = static_cast<std::uint16_t>(itemKind);
    } else if ((GetCaps() & 4) != 0) {
        auto* inventoryItem = reinterpret_cast<std::uint16_t*>(m_pBaseInventory->GetInventoryItem(itemPosition));
        m_entries[m_itemsNum].itemKind = inventoryItem ? *inventoryItem : 0;
    }

    m_entries[m_itemsNum].itemQty = static_cast<std::uint16_t>(itemQuantity);
    m_entries[m_itemsNum].itemPos = itemPosition;
    m_entries[m_itemsNum].value0 = static_cast<std::uint32_t>(value0);
    m_entries[m_itemsNum].value1 = static_cast<std::uint32_t>(sealedStatus);

    if (outIndex) {
        *outIndex = m_itemsNum;
    }

    ++m_itemsNum;
    return 1;
}

int cltItemList::AddItemToBasket(std::uint16_t itemKind, std::uint16_t itemQuantity, std::uint16_t itemPosition, int* outIndex) {
    int matchedIndex;

    if (m_itemsNum >= 0x1F4u) {
        return 0;
    }

    matchedIndex = 0;
    if ((GetCaps() & 1) != 0) {
        const int currentItemCount = m_itemsNum;
        if (static_cast<std::uint16_t>(currentItemCount)) {
            auto* itemKindCursor = reinterpret_cast<std::uint16_t*>(&m_entries[0]);
            while (*itemKindCursor != itemKind) {
                ++matchedIndex;
                itemKindCursor += 10;
                if (matchedIndex >= currentItemCount) {
                    goto LABEL_19;
                }
            }
            m_entries[matchedIndex].itemQty = static_cast<std::uint16_t>(m_entries[matchedIndex].itemQty + itemQuantity);
            if (outIndex) {
                *outIndex = matchedIndex;
                goto LABEL_19;
            }
        }
    } else if ((GetCaps() & 4) != 0) {
        matchedIndex = 0;
        const int currentItemCount = m_itemsNum;
        if (static_cast<std::uint16_t>(currentItemCount)) {
            auto* itemPositionCursor = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 8);
            while (*itemPositionCursor != itemPosition) {
                ++matchedIndex;
                itemPositionCursor += 10;
                if (matchedIndex >= currentItemCount) {
                    goto LABEL_19;
                }
            }
            m_entries[matchedIndex].itemQty = static_cast<std::uint16_t>(m_entries[matchedIndex].itemQty + itemQuantity);
            if (outIndex) {
                *outIndex = matchedIndex;
            }
        }
    }

LABEL_19:
    if (matchedIndex == m_itemsNum) {
        if ((GetCaps() & 1) != 0) {
            AddItem(
                static_cast<std::int16_t>(itemKind),
                static_cast<std::int16_t>(itemQuantity),
                0,
                0,
                m_itemsNum,
                reinterpret_cast<std::uint32_t*>(outIndex));
            return 1;
        }
        if ((GetCaps() & 4) != 0) {
            AddItem(
                static_cast<std::int16_t>(itemKind),
                static_cast<std::int16_t>(itemQuantity),
                0,
                0,
                itemPosition,
                reinterpret_cast<std::uint32_t*>(outIndex));
        }
    }
    return 1;
}

int cltItemList::DeleteItem(std::uint16_t a2, std::uint16_t a3, std::uint16_t a4) {
    const std::uint16_t v5 = m_itemsNum;
    if (v5 >= 0x1F4u) {
        return 0;
    }

    int v6 = 0;
    if (!v5) {
        return 0;
    }

    auto* v7 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 8);
    while ((GetCaps() & 1) != 0) {
        if (*(v7 - 2) == a2) {
            const std::int16_t v9 = static_cast<std::int16_t>(m_entries[v6].itemQty);
            auto* v10 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 20 * v6);
            const std::uint16_t v11 = a3;
            if (v9 == static_cast<std::int16_t>(a3)) {
                const int v12 = 4 * (5 * v6 + 5);
                std::memmove(v10 + 2, v10 + 12, 20 * m_itemsNum - v12);
                --m_itemsNum;
                return 1;
            }
            v10[3] = static_cast<std::uint16_t>(v9 - v11);
            return 1;
        }

        ++v6;
        v7 += 10;
        if (v6 >= m_itemsNum) {
            return 0;
        }
    }

    if ((GetCaps() & 4) == 0 || *v7 != a4) {
        ++v6;
        v7 += 10;
        if (v6 >= m_itemsNum) {
            return 0;
        }
    }

    const std::uint16_t v11 = a3;
    auto* v10 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 20 * v6);
    const std::int16_t v9 = static_cast<std::int16_t>(v10[3]);
    if (v9 == static_cast<std::int16_t>(a3)) {
        const int v12 = 4 * (5 * v6 + 5);
        std::memmove(v10 + 2, v10 + 12, 20 * m_itemsNum - v12);
        --m_itemsNum;
        return 1;
    }

    v10[3] = static_cast<std::uint16_t>(v9 - v11);
    return 1;
}

int cltItemList::DelLastItem() {
    const std::int16_t v1 = static_cast<std::int16_t>(m_itemsNum);
    if (!v1) {
        return 0;
    }
    m_itemsNum = static_cast<std::uint16_t>(v1 - 1);
    return 1;
}

int cltItemList::GetItem(unsigned int a2, std::uint16_t* a3, std::uint16_t* a4, unsigned int* a5, std::uint16_t* a6, std::uint16_t* a7) {
    if (a2 < m_itemsNum) {
        if (a3) {
            *a3 = m_entries[a2].itemKind;
        }
        if (a4) {
            *a4 = m_entries[a2].itemQty;
        }
        if (a5) {
            *a5 = m_entries[a2].value0;
        }
        if (a7) {
            *a7 = m_entries[a2].tradeItemPos;
        }
        if (a6) {
            *a6 = m_entries[a2].itemPos;
        }
        return 1;
    }

    *a3 = 0;
    *a4 = 0;
    return 0;
}

int cltItemList::GetSealedStatus(unsigned int a2) {
    if (a2 < m_itemsNum) {
        return m_entries[a2].value1;
    }
    return 0;
}

unsigned int cltItemList::SetSealedStatus(unsigned int a2, int a3) {
    unsigned int result = a2;
    if (a2 < m_itemsNum) {
        result = 5 * a2;
        m_entries[a2].value1 = static_cast<std::uint32_t>(a3);
    }
    return result;
}

std::uint16_t cltItemList::GetItemKind(unsigned int a2) {
    if (a2 < m_itemsNum) {
        return m_entries[a2].itemKind;
    }
    return 0;
}

std::uint16_t cltItemList::GetItemsNum() { return m_itemsNum; }

void cltItemList::FillOutItemListInfo(CMofMsg* message) {
    message->Put_WORD(m_itemsNum);
    int itemIndex = 0;
    if (m_itemsNum) {
        auto* itemDataCursor = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);
        do {
            if ((GetCaps() & 1) != 0) {
                message->Put_WORD(*(itemDataCursor - 1));
            }
            if ((GetCaps() & 2) != 0) {
                message->Put_WORD(*itemDataCursor);
            }
            if ((GetCaps() & 4) != 0) {
                message->Put_WORD(itemDataCursor[1]);
            }
            ++itemIndex;
            itemDataCursor += 10;
        } while (itemIndex < m_itemsNum);
    }
}

int cltItemList::GetAllItemsPrice(std::uint16_t a2, int a3) {
    int v4 = 0;
    int v5 = 0;
    std::uint64_t v18 = 0;
    strPackageShopInfo* v16 = nullptr;
    stShopInfo* v17 = nullptr;

    int result = m_pclShopInfo->GetShopInfoByID(a2, &v17, &v16);
    if (result) {
        if (v16) {
            if (m_itemsNum) {
                auto* v7 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 4);
                int v10 = 0;
                do {
                    std::int64_t v8 = m_pclShopInfo->GetPackageItemPrice(a2, *v7, v7[1]);
                    std::int64_t v9 = GetItemPrice(v8, 1000, a3);
                    v10 = static_cast<int>(v9) + v5;
                    const std::int64_t high = (v9 + (static_cast<std::uint64_t>(static_cast<std::uint32_t>(v5)) | (v18 & 0xFFFFFFFF00000000ULL))) >> 32;
                    v5 = v10;
                    const std::uint16_t v11 = m_itemsNum;
                    ++v4;
                    v7 += 10;
                    v18 = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(high)) << 32) | static_cast<std::uint32_t>(v18);
                    if (v4 >= v11) {
                        return v10;
                    }
                } while (true);
            }
        } else if (v17) {
            const int v19 = *(reinterpret_cast<int*>(v17) + 1);
            if (m_itemsNum) {
                auto* v12 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);
                do {
                    stItemKindInfo* v13 = m_pclItemKindInfo->GetItemKindInfo(*(v12 - 1));
                    const std::int64_t v14 = GetItemPrice(*(reinterpret_cast<unsigned int*>(v13) + 2), v19, a3) * *v12;
                    const int v15 = static_cast<int>(v14 + v18);
                    v5 = v15;
                    ++v4;
                    v12 += 10;
                    v18 = static_cast<std::uint64_t>(v14 + v18);
                } while (v4 < m_itemsNum);
            }
        }
        result = v5;
    }
    return result;
}

int cltItemList::GetAllItemGoldCoinPrice(stShopInfo* a2) {
    int v3 = 0;
    strPackageShopInfo* v8 = nullptr;
    int result = m_pclShopInfo->GetShopInfoByID(static_cast<std::uint16_t>(reinterpret_cast<std::uintptr_t>(a2)), &a2, &v8);
    if (result) {
        if (a2) {
            int v5 = 0;
            if (m_itemsNum) {
                auto* v6 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);
                do {
                    stItemKindInfo* v7 = m_pclItemKindInfo->GetItemKindInfo(*(v6 - 1));
                    if (v7) {
                        v3 += *v6 * *(reinterpret_cast<int*>(v7) + 4);
                    }
                    ++v5;
                    v6 += 10;
                } while (v5 < m_itemsNum);
            }
        }
        result = v3;
    }
    return result;
}

int cltItemList::GetAllItemSilverCoinPrice(stShopInfo* a2) {
    int v3 = 0;
    strPackageShopInfo* v8 = nullptr;
    int result = m_pclShopInfo->GetShopInfoByID(static_cast<std::uint16_t>(reinterpret_cast<std::uintptr_t>(a2)), &a2, &v8);
    if (result) {
        if (a2) {
            int v5 = 0;
            if (m_itemsNum) {
                auto* v6 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);
                do {
                    stItemKindInfo* v7 = m_pclItemKindInfo->GetItemKindInfo(*(v6 - 1));
                    if (v7) {
                        v3 += *v6 * *(reinterpret_cast<int*>(v7) + 5);
                    }
                    ++v5;
                    v6 += 10;
                } while (v5 < m_itemsNum);
            }
        }
        result = v3;
    }
    return result;
}

int cltItemList::GetAllItemBronzeCoinPrice(stShopInfo* a2) {
    int v3 = 0;
    strPackageShopInfo* v8 = nullptr;
    int result = m_pclShopInfo->GetShopInfoByID(static_cast<std::uint16_t>(reinterpret_cast<std::uintptr_t>(a2)), &a2, &v8);
    if (result) {
        if (a2) {
            int v5 = 0;
            if (m_itemsNum) {
                auto* v6 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);
                do {
                    stItemKindInfo* v7 = m_pclItemKindInfo->GetItemKindInfo(*(v6 - 1));
                    if (v7) {
                        v3 += *v6 * *(reinterpret_cast<int*>(v7) + 6);
                    }
                    ++v5;
                    v6 += 10;
                } while (v5 < m_itemsNum);
            }
        }
        result = v3;
    }
    return result;
}

int cltItemList::GetAllItemsPriceForSell(int a2) {
    int v3 = 0;
    int result = 0;
    std::int64_t v9 = 0;

    if (m_itemsNum) {
        auto* v5 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);
        do {
            stItemKindInfo* v6 = m_pclItemKindInfo->GetItemKindInfo(*(v5 - 1));
            const int v7 = a2 ? (1000 - a2) : 500;
            const std::int64_t v8 = *(reinterpret_cast<unsigned int*>(v6) + 2) * static_cast<std::int64_t>(v7) / 1000 * *v5 + v9;
            ++v3;
            v5 += 10;
            v9 = v8;
        } while (v3 < m_itemsNum);
        result = static_cast<int>(v9);
    }
    return result;
}

int cltItemList::GetAllItemsBuyPVPPoint() {
    int v2 = 0;
    int v3 = 0;
    if (!m_itemsNum) {
        return v3;
    }

    for (auto* i = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(this) + 6);; i += 10) {
        stItemKindInfo* v5 = m_pclItemKindInfo->GetItemKindInfo(*(i - 1));
        if (!v5) {
            break;
        }
        const int v6 = *(reinterpret_cast<int*>(v5) + 3);
        if (!v6) {
            break;
        }
        v3 += v6 * *i;
        if (++v2 >= m_itemsNum) {
            return v3;
        }
    }
    return 0;
}

void cltItemList::SetCaps(unsigned int a2) { m_caps = a2; }

unsigned int cltItemList::GetCaps() { return m_caps; }

void cltItemList::SetBaseInventory(cltBaseInventory* a2) { m_pBaseInventory = a2; }

void cltItemList::Normalize() {
    unsigned int v1 = 0;
    unsigned int v8 = 0;
    if (GetItemsNum()) {
        do {
            std::uint16_t v5 = 0;
            std::uint16_t v7 = 0;
            GetItem(v1, &v5, &v7, &v8, nullptr, nullptr);
            int v3 = v1 + 1;
            for (int i = v1 + 1; i < GetItemsNum(); ++i) {
                std::uint16_t v6 = 0;
                std::uint16_t v9[2]{};
                GetItem(i, &v6, v9, &v8, nullptr, nullptr);
                if (v5 == v6) {
                    AddItemQty(v1, v9[0]);
                    DeleteItem(static_cast<unsigned int>(i--));
                }
            }
            ++v1;
            if (v3 >= GetItemsNum()) {
                break;
            }
        } while (true);
    }
}

int cltItemList::AddItemQty(unsigned int a2, std::uint16_t a3) {
    if (a2 >= m_itemsNum) {
        return 0;
    }
    m_entries[a2].itemQty = static_cast<std::uint16_t>(m_entries[a2].itemQty + a3);
    return 1;
}

int cltItemList::DeleteItem(unsigned int a2) {
    const int v3 = m_itemsNum;
    if (a2 >= static_cast<std::uint16_t>(v3)) {
        return 0;
    }
    std::memmove(
        reinterpret_cast<char*>(this) + 20 * a2 + 4,
        reinterpret_cast<char*>(this) + 20 * a2 + 24,
        20 * v3 - 20 * static_cast<int>(a2) - 20);
    --m_itemsNum;
    return 1;
}

void cltItemList::TranslateQueryArg(char* const buffer) {
    char* queryCursor = buffer;
    int itemIndex = 0;
    *buffer = 0;
    if (m_itemsNum) {
        do {
            std::uint16_t itemKind = 0;
            std::uint16_t itemQuantity = 0;
            std::uint16_t itemPosition = 0;
            unsigned int extraValue = 0;
            GetItem(itemIndex, &itemKind, &itemQuantity, &extraValue, &itemPosition, nullptr);
            std::sprintf(queryCursor, "%s%d, %d, %d, ", queryCursor, itemPosition, itemKind, itemQuantity);
            ++itemIndex;
        } while (itemIndex < m_itemsNum);
    }
}

void cltItemList::GetItemInfo(std::uint16_t itemIndex, std::uint16_t* outItemKind, std::uint16_t* outItemPosition, std::uint16_t* outItemQuantity) {
    unsigned int ignoredValue = 0;
    GetItem(itemIndex, outItemKind, outItemQuantity, &ignoredValue, outItemPosition, nullptr);
}

void cltItemList::AddTradeItemPos(std::uint16_t a2, std::uint16_t a3) {
    m_entries[a2].tradeItemPos = a3;
}

std::int64_t cltItemList::GetItemPrice(std::int64_t a1, int a2, int a3) {
    return a2 * a1 / 1000 - a3 * (a2 * a1 / 1000) / 1000;
}
