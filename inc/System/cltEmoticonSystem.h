#pragma once

#include <array>
#include <cstdint>

struct stEmoticonItemInfo;
class cltBaseInventory;
class cltEmoticonKindInfo;

class cltEmoticonSystem {
public:
    cltEmoticonSystem();
    virtual ~cltEmoticonSystem();

    static void InitializeStaticVariable(cltBaseInventory* baseInventory, cltEmoticonKindInfo* emoticonKindInfo);

    int Initialize(std::uint8_t* shortcutData);
    int AutoReload();

    stEmoticonItemInfo* GetBuyEmoticonItemInfoByIndex(std::uint8_t index);
    int AddShortCutData(std::uint8_t index, int data);
    int DelShortCutData(std::uint8_t index);
    int MoveShortCutData(std::uint8_t from, std::uint8_t to);
    int GetData(std::uint8_t index);

    void AddEmoticonItem(std::uint16_t kind);
    void DelEmoticonItem(std::uint16_t kind);

protected:
    std::array<std::uint16_t, 120> m_kindSlots{};
    std::uint8_t m_ownedSetCount = 0;
    std::array<int, 5> m_shortCut{};

    static cltBaseInventory* m_pcltBaseInventory;
    static cltEmoticonKindInfo* m_pcltEmoticonKindInfo;
};
