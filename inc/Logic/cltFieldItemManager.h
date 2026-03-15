#pragma once
#include <cstdint>

class cltFieldItemManager {
public:
    cltFieldItemManager() = default;
    ~cltFieldItemManager() = default;

    // Pushes a field-item drop buffer entry.
    // account  : owner account
    // dropId   : drop identifier
    // a3       : reserved
    // itemKind : item kind code
    // itemQty  : quantity
    // a6       : reserved
    void PushBuffer(unsigned int account, unsigned int dropId, int a3,
                    unsigned short itemKind, int itemQty, int a6);

    // Returns information about the nearest field item to (x, y).
    // outKind  : item kind of nearest item (0 if none)
    // outX/Y   : position of nearest item
    // a5-a7    : reserved filter/flag parameters
    void GetNearItemInfo(float x, float y,
                         std::uint16_t* outKind, float* outX, float* outY,
                         int a5, int a6, int a7);

    // Removes an item from the field.
    // account  : owner account
    // dropId   : drop identifier
    // a3, a4   : reserved
    void DelItem(unsigned int account, unsigned int dropId, int a3, int a4);
};

extern cltFieldItemManager g_clFieldItemMgr;
extern int g_nFieldItemPickupFlag;
