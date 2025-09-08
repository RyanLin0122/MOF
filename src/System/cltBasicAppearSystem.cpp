#include "System/cltBasicAppearSystem.h"

cltBasicAppearSystem::cltBasicAppearSystem() {
    // 反編譯建構子：僅將 *((DWORD*)this + 1) 置 0，其餘保持預設 0
    hairColorKey_ = 0;
}

void cltBasicAppearSystem::Initialize(uint8_t hair, uint8_t face, uint32_t key) {
    hair_ = hair;
    face_ = face;
    hairColorKey_ = key;
}

uint8_t cltBasicAppearSystem::GetHair() const {
    return hair_;
}

uint8_t cltBasicAppearSystem::GetFace() const {
    return face_;
}

uint32_t cltBasicAppearSystem::GetHairColorKey() const {
    return hairColorKey_;
}

void cltBasicAppearSystem::SetNewHair(uint8_t h) {
    hair_ = h;
}

void cltBasicAppearSystem::SetNewFace(uint8_t f) {
    face_ = f;
}

void cltBasicAppearSystem::SetNewHairColorKey(uint32_t k) {
    hairColorKey_ = k;
}

void cltBasicAppearSystem::ResetHairColorKey() {
    hairColorKey_ = 0;
}
