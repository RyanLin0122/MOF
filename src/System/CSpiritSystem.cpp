#include "System/CSpiritSystem.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "global.h"

namespace {
bool IsDigit(const char* s) {
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

CSpiritSystem::CSpiritSystem() { m_infoCount = 0; }

CSpiritSystem::~CSpiritSystem() {
    for (auto*& info : m_infos) {
        delete info;
        info = nullptr;
    }
    m_infoCount = 0;
}

int CSpiritSystem::Initialize(char* filename) {
    char delimiter[3] = "\t\n";
    int count = 0;
    int success = 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char buffer[1024]{};
    if (std::fgets(buffer, sizeof(buffer), fp) && std::fgets(buffer, sizeof(buffer), fp) && std::fgets(buffer, sizeof(buffer), fp)) {
        if (std::fgets(buffer, sizeof(buffer), fp)) {
            while (true) {
                char* tok = std::strtok(buffer, delimiter);
                if (!tok) break;

                if (count >= static_cast<int>(m_infos.size())) {
                    break;
                }

                const std::uint16_t kind = TranslateKindCode(tok);
                if (m_infos[static_cast<std::uint16_t>(count)] != nullptr) {
                    break;
                }

                auto* info = new stSpiritInfo{};
                m_infos[static_cast<std::uint16_t>(count)] = info;
                info->wSpiritId = kind;

                if (!std::strtok(nullptr, delimiter)) break;

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wNameTextCode = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wDescTextCode = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wNeedLevelFrom = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wNeedLevelTo = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok) break;
                info->wNeedItemId = TranslateKindCode(tok);

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wSpiritAttribute = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wIncStr = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wIncInt = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wIncDex = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wIncVit = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsAlphaNumeric(tok)) break;
                std::sscanf(tok, "%x", &info->dwResourceId);

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wStartBlockId = static_cast<std::uint16_t>(std::atoi(tok));

                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok)) break;
                info->wBlockCount = static_cast<std::uint16_t>(std::atoi(tok));

                ++count;
                if (!std::fgets(buffer, sizeof(buffer), fp)) {
                    success = 1;
                    m_infoCount = static_cast<std::uint16_t>(count);
                    break;
                }
            }
        } else {
            success = 1;
            m_infoCount = 0;
        }
    }

    g_clTextFileManager.fclose(fp);
    return success;
}

std::uint16_t CSpiritSystem::GetSpiritKind(std::uint16_t level) {
    if (!m_infoCount) return 0;
    for (std::uint16_t i = 0; i < m_infoCount; ++i) {
        auto* info = m_infos[i];
        if (info && info->wNeedLevelFrom <= level && info->wNeedLevelTo >= level) {
            return info->wSpiritId;
        }
    }
    return 0;
}

stSpiritInfo* CSpiritSystem::GetSpiritInfo(std::uint16_t spiritKind, std::uint16_t level) {
    if (!m_infoCount) return nullptr;
    for (std::uint16_t i = 0; i < m_infoCount; ++i) {
        auto* info = m_infos[i];
        if (info && info->wNeedItemId == spiritKind && info->wNeedLevelFrom <= level && info->wNeedLevelTo >= level) {
            return info;
        }
    }
    return nullptr;
}

stSpiritInfo* CSpiritSystem::GetSpiritInfo(std::uint16_t kind) {
    if (!m_infoCount) return nullptr;
    for (std::uint16_t i = 0; i < m_infoCount; ++i) {
        auto* info = m_infos[i];
        if (info && info->wSpiritId == kind) return info;
    }
    return nullptr;
}

std::uint16_t CSpiritSystem::TranslateKindCode(char* s) {
    if (!s || std::strlen(s) != 5) return 0;
    const int prefix = (std::toupper(static_cast<unsigned char>(*s)) + 31) << 11;
    const auto number = static_cast<std::uint16_t>(std::atoi(s + 1));
    if (number >= 0x800u) return 0;
    return static_cast<std::uint16_t>(prefix | number);
}
