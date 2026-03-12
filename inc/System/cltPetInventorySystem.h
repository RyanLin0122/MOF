#pragma once

#include <cstdint>

class CMofMsg;

class cltPetInventorySystem {
public:
    void Initialize(CMofMsg* msg);
    void Free();
    void OnPetCreated(std::uint8_t bagInitCount);
    void OnPetDeleted();

    int CanMoveItem(std::uint8_t from, std::uint8_t to);
    void MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* outChanged);
    int IsPetInventoryEmpty();

    std::uint8_t GetPetBagNum();
    int CanIncreasePetBagNum();
    void IncreasePetBagNum();
};

