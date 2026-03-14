#include "System/CSpiritSystem.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "global.h"

namespace {
bool IsDigitStr(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p) {
        if (!std::isdigit(*p)) return false;
    }
    return true;
}

bool IsAlphaNumeric(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p) {
        if (!std::isalnum(*p)) return false;
    }
    return true;
}
} // namespace

CSpiritSystem::CSpiritSystem() = default;
CSpiritSystem::~CSpiritSystem() = default;

int CSpiritSystem::Initialize(char* filename) {
    m_infos.clear();

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[1024]{};
    const char* delimiter = "\t\n";

    if (!std::fgets(line, sizeof(line), fp) || !std::fgets(line, sizeof(line), fp) || !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    while (std::fgets(line, sizeof(line), fp)) {
        stSpiritInfo info{};
        char* tok = std::strtok(line, delimiter);
        if (!tok) break;
        info.wKind = TranslateKindCode(tok);

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wTextCode = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wGrade = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wFromLevel = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wToLevel = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok) break;
        info.wSpiritKind = TranslateKindCode(tok);

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wUnknown6 = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wUnknown7 = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wUnknown8 = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wUnknown9 = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wUnknown10 = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsAlphaNumeric(tok)) break;
        std::sscanf(tok, "%hx", &info.wResId);

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wBlockId = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delimiter);
        if (!tok || !IsDigitStr(tok)) break;
        info.wTooltipId = static_cast<std::uint16_t>(std::atoi(tok));

        m_infos.push_back(info);
    }

    g_clTextFileManager.fclose(fp);
    return 1;
}

std::uint16_t CSpiritSystem::GetSpiritKind(std::uint16_t level) {
    for (const auto& info : m_infos) {
        if (info.wFromLevel <= level && info.wToLevel >= level) {
            return info.wKind;
        }
    }
    return 0;
}

stSpiritInfo* CSpiritSystem::GetSpiritInfo(std::uint16_t spiritKind, std::uint16_t level) {
    for (auto& info : m_infos) {
        if (info.wSpiritKind == spiritKind && info.wFromLevel <= level && info.wToLevel >= level) {
            return &info;
        }
    }
    return nullptr;
}

stSpiritInfo* CSpiritSystem::GetSpiritInfo(std::uint16_t kind) {
    for (auto& info : m_infos) {
        if (info.wKind == kind) return &info;
    }
    return nullptr;
}

std::uint16_t CSpiritSystem::TranslateKindCode(char* s) {
    if (!s || std::strlen(s) != 5) return 0;
    const int prefix = (std::toupper(*s) + 31) << 11;
    const auto number = static_cast<std::uint16_t>(std::atoi(s + 1));
    if (number >= 0x800u) return 0;
    return static_cast<std::uint16_t>(prefix | number);
}
