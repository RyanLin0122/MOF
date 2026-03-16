#pragma once

#include <cstdint>

class cltBaseInventory;
class cltMoneySystem;
class cltEmblemSystem;
class cltEnchantKindInfo;

class cltEnchantSystem {
public:
    static void InitializeStaticVariable(cltEnchantKindInfo* pEnchantKindInfo);

    cltEnchantSystem();
    ~cltEnchantSystem();

    void Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, cltEmblemSystem* pEmblem);
    void Free();

    int CanEnchantItem(std::uint8_t weaponSlot, std::uint8_t stoneSlot);
    int EnchantItem(unsigned int seed, std::uint8_t weaponSlot, char stoneSlot,
                    std::uint8_t* changeSlots, std::uint16_t* outEnchantID,
                    int* outCost, std::uint16_t* outResultItemKind, std::uint16_t* outStoneKind);
    std::uint16_t GetEnchantItemKind(std::uint8_t weaponSlot);

private:
    cltBaseInventory* m_pInventory = nullptr;
    cltMoneySystem* m_pMoney = nullptr;
    cltEmblemSystem* m_pEmblem = nullptr;

    static cltEnchantKindInfo* m_pclEnchantKindInfo;
};
