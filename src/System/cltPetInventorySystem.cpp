#include "System/cltPetInventorySystem.h"

#include "Network/CMofMsg.h"

void cltPetInventorySystem::Initialize(CMofMsg*) {}
void cltPetInventorySystem::Free() {}
void cltPetInventorySystem::OnPetCreated(std::uint8_t) {}
void cltPetInventorySystem::OnPetDeleted() {}

int cltPetInventorySystem::CanMoveItem(std::uint8_t, std::uint8_t) { return 0; }
void cltPetInventorySystem::MoveItem(std::uint8_t, std::uint8_t, std::uint8_t* outChanged) {
    if (outChanged) *outChanged = 0;
}
int cltPetInventorySystem::IsPetInventoryEmpty() { return 1; }

std::uint8_t cltPetInventorySystem::GetPetBagNum() { return 0; }
int cltPetInventorySystem::CanIncreasePetBagNum() { return 0; }
void cltPetInventorySystem::IncreasePetBagNum() {}

