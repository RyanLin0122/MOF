#include "Character/CCA.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

namespace {
constexpr std::size_t kLayerBase = 24;
constexpr std::size_t kLayerCount = 23;
constexpr std::size_t kEquipBase = 160;
constexpr std::size_t kHairColorBase = 260;
constexpr std::size_t kSexOffset = 248;
constexpr std::size_t kHairOffset = 252;
constexpr std::size_t kFaceOffset = 256;

inline std::uint16_t& U16(char* base, std::size_t off) {
    return *reinterpret_cast<std::uint16_t*>(base + off);
}
inline std::uint32_t& U32(char* base, std::size_t off) {
    return *reinterpret_cast<std::uint32_t*>(base + off);
}
inline float& F32(char* base, std::size_t off) {
    return *reinterpret_cast<float*>(base + off);
}

inline void SetDefaultHairColor(char* base) {
    F32(base, kHairColorBase + 0) = 1.0f;
    F32(base, kHairColorBase + 4) = 1.0f;
    F32(base, kHairColorBase + 8) = 1.0f;
    F32(base, kHairColorBase + 12) = 1.0f;
}
}

CCA::CCA() {
    std::memset(m_data, 0, sizeof(m_data));
    SetDefaultHairColor(m_data);
}

CCA::~CCA() {
}

void CCA::LoadCA(const char* /*path*/, CEffectBase** a2, CEffectBase** a3) {
    std::fill_n(reinterpret_cast<std::uint32_t*>(m_data + kLayerBase), kLayerCount, 0U);
    U32(m_data, 240) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(a2));
    U32(m_data, 244) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(a3));
    U32(m_data, 284) = 0; // transport inactive
}

void CCA::Play(int motion, bool /*loop*/) {
    U32(m_data, 144) = static_cast<std::uint32_t>(motion);
}

void CCA::Process() {
}

void CCA::Draw() {
}

void CCA::InitItem(unsigned char sex, unsigned short hair, unsigned short face, unsigned int hairColor) {
    U32(m_data, kSexOffset) = sex;
    U32(m_data, kHairOffset) = hair;
    U32(m_data, kFaceOffset) = face;

    for (std::size_t i = 0; i < 16; ++i) {
        U32(m_data, kLayerBase + i * 4) = 0;
    }

    if (hairColor == 0) {
        SetDefaultHairColor(m_data);
        return;
    }

    F32(m_data, kHairColorBase + 0) = static_cast<float>((hairColor >> 16) & 0xFF) / 255.0f;
    F32(m_data, kHairColorBase + 4) = static_cast<float>((hairColor >> 8) & 0xFF) / 255.0f;
    F32(m_data, kHairColorBase + 8) = static_cast<float>(hairColor & 0xFF) / 255.0f;
    F32(m_data, kHairColorBase + 12) = static_cast<float>((hairColor >> 24) & 0xFF) / 255.0f;
}

void CCA::ResetItem(unsigned char sex, unsigned short hair, unsigned short face, unsigned char a5) {
    for (std::size_t i = 0; i < 16; ++i) {
        const std::size_t off = kEquipBase + (i * 2) + (a5 ? 2 : 0);
        const auto itemId = U16(m_data, off);
        if (itemId) {
            SetItemID(itemId, sex, 0, hair, face, a5);
        }
    }

    if (U16(m_data, kEquipBase + 2) == 0) {
        U16(m_data, kEquipBase + 0) = hair;
    }
    U32(m_data, kFaceOffset) = face;
}

void CCA::SetItemID(unsigned short itemId, unsigned char sex, int a4, int a5, int a6, unsigned char a7) {
    const std::uint16_t slot = static_cast<std::uint16_t>(itemId & 0xF);
    U32(m_data, kSexOffset) = sex;
    U32(m_data, 252) = static_cast<std::uint32_t>(a5);
    U32(m_data, 256) = static_cast<std::uint32_t>(a6);

    const std::size_t equipOff = kEquipBase + slot * 2 + (a7 ? 2 : 0);
    U16(m_data, equipOff) = a4 ? itemId : 0;

    if (slot < kLayerCount) {
        U32(m_data, kLayerBase + slot * 4) = a4 ? itemId : 0;
    }
}

void CCA::BegineEmoticon(int a2) {
    U32(m_data, kLayerBase + 12 * 4) = static_cast<std::uint32_t>(a2);
    U32(m_data, kLayerBase + 22 * 4) = static_cast<std::uint32_t>(a2 + 1);
}

void CCA::EndEmoticon(unsigned short a2, unsigned char /*a3*/) {
    U32(m_data, kLayerBase + 1 * 4) = a2;
    U32(m_data, kLayerBase + 22 * 4) = 0;
}

unsigned char ExGetIllustCharSexCode(char charKind) {
    return static_cast<unsigned char>(charKind & 1);
}
