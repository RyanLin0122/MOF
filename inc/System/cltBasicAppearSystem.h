#pragma once
#include <cstdint>

class cltBasicAppearSystem {
public:
    cltBasicAppearSystem();                                      // atb=0
    void Initialize(uint8_t hair, uint8_t face, uint32_t key);   // 對應 __thiscall Initialize

    // getters
    uint8_t  GetHair() const;
    uint8_t  GetFace() const;
    uint32_t GetHairColorKey() const;

    // setters
    void SetNewHair(uint8_t h);
    void SetNewFace(uint8_t f);
    void SetNewHairColorKey(uint32_t k);
    void ResetHairColorKey(); // 設為 0

private:
    // 使 hairColorKey 位於 offset 4（符合反編譯：*((DWORD*)this + 1)）
    uint8_t  hair_{ 0 };
    uint8_t  face_{ 0 };
    uint16_t pad_{ 0 };          // 對齊用
    uint32_t hairColorKey_{ 0 }; // offset 4
};
