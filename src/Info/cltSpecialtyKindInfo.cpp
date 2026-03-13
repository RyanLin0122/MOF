#include "Info/cltSpecialtyKindInfo.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

std::uint16_t cltSpecialtyKindInfo::TranslateKindCode(char* s) {
    if (!s || std::strlen(s) != 5) return 0;

    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const int lo = std::atoi(s + 1);
    if (lo < 0x800) return static_cast<std::uint16_t>(hi | lo);
    return 0;
}

strSpecialtyKindInfo* cltSpecialtyKindInfo::GetSpecialtyKindInfo(std::uint16_t kind) {
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [kind](const strSpecialtyKindInfo& e) { return e.wKind == kind; });
    return it == m_entries.end() ? nullptr : &(*it);
}

int cltSpecialtyKindInfo::GetSpecialtyList(char category, std::uint16_t* outList) {
    int count = 0;
    for (const auto& e : m_entries) {
        if (category == 0 || e.wCategory == static_cast<std::uint16_t>(static_cast<unsigned char>(category))) {
            if (outList) outList[count] = e.wKind;
            ++count;
        }
    }
    return count;
}

int cltSpecialtyKindInfo::IsLastLevelSpecialty(std::uint16_t kind) {
    auto* info = GetSpecialtyKindInfo(kind);
    if (!info) return 0;
    for (const auto& e : m_entries) {
        if (e.wRequiredSpecialtyKind == kind) return 0;
    }
    return 1;
}

int cltSpecialtyKindInfo::IsGenericSpeciatly(std::uint16_t kind) {
    auto* info = GetSpecialtyKindInfo(kind);
    return info ? (info->byType == 0) : 0;
}

int cltSpecialtyKindInfo::IsMakingItemSpecialty(std::uint16_t kind) {
    auto* info = GetSpecialtyKindInfo(kind);
    return info ? (info->byType == 1) : 0;
}

int cltSpecialtyKindInfo::IsTransformSpecialty(std::uint16_t kind) {
    auto* info = GetSpecialtyKindInfo(kind);
    return info ? (info->byType == 2) : 0;
}

int cltSpecialtyKindInfo::IsCircleSpecialty(std::uint16_t kind) {
    auto* info = GetSpecialtyKindInfo(kind);
    return info ? (info->byIsCircle != 0) : 0;
}

std::int16_t cltSpecialtyKindInfo::GetSpecialtyTotalPoint(std::uint16_t kind) {
    std::int16_t result = 0;
    auto* cur = GetSpecialtyKindInfo(kind);
    while (cur) {
        result = static_cast<std::int16_t>(result + cur->byRequiredSpecialtyPt);
        if (!cur->wRequiredSpecialtyKind) break;
        cur = GetSpecialtyKindInfo(cur->wRequiredSpecialtyKind);
    }
    return result;
}

void cltSpecialtyKindInfo::SetSpecialtyTable(std::vector<strSpecialtyKindInfo> entries) {
    m_entries = std::move(entries);
}
