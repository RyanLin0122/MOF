#include "Info/cltInstantDungeonKindInfo.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>

#ifdef _WIN32
#include <windows.h>
#endif

cltInstantDungeonKindInfo::cltInstantDungeonKindInfo()
    : m_data(nullptr)
    , m_count(0)
{
}

cltInstantDungeonKindInfo::~cltInstantDungeonKindInfo()
{
    Free();
}

int cltInstantDungeonKindInfo::Initialize(char* fileName)
{
    char delimiter[3];
    FILE* stream = nullptr;
    int result = 0;
    std::fpos_t position;
    char buffer[1024];

    std::memset(buffer, 0, sizeof(buffer));
    std::strcpy(delimiter, "\t\n");

    stream = g_clTextFileManager.fopen(fileName);
    if (!stream)
    {
        return result;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream))
    {
        std::fgetpos(stream, &position);

        for (; std::fgets(buffer, 1023, stream); ++m_count)
        {
        }

        if (!m_count)
        {
            g_clTextFileManager.fclose(stream);
            return 1;
        }

        m_data = static_cast<strInstantDungeonKindInfo*>(::operator new(sizeof(strInstantDungeonKindInfo) * m_count));
        std::memset(m_data, 0, sizeof(strInstantDungeonKindInfo) * m_count);

        std::fsetpos(stream, &position);

        char* line = std::fgets(buffer, 1023, stream);
        auto* current = reinterpret_cast<unsigned char*>(m_data);
        if (line)
        {
            while (true)
            {
                char* token = std::strtok(buffer, delimiter); // 1 wIndunType
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 0) = TranslateKindCode(token);

                if (!std::strtok(nullptr, delimiter)) // 2 PlannerName, only consumed
                {
                    goto EXIT_LABEL;
                }

                token = std::strtok(nullptr, delimiter); // 3 wIndunNameTextCode
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 4) = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 4 wBaseIndunId
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 2) = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 5 bLevelLimit
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint8_t*>(current + 6) = static_cast<std::uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 6 wItemIdForGeneration
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 8) = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 7 wNpcId
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 10) = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 8 wBossMonsterId
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 12) = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 9 wRandomMonsterSpawn
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 14) = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 10 wSlowMonsterId
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 16) = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 11 wSlowMonsterProbability
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 18) = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 12 wPowerManMonsterId
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 20) = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 13 wPowerManMonsterProbability
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 22) = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 14 wIndunGuideTextCode
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint16_t*>(current + 24) = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 15 dwTimeLimit
                if (!token)
                {
                    goto EXIT_LABEL;
                }
                *reinterpret_cast<std::uint32_t*>(current + 28) = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 16 LoadingResourceId
                if (!token || !IsAlphaNumeric(token))
                {
                    goto EXIT_LABEL;
                }

                if (std::sscanf(token, "%x", reinterpret_cast<unsigned int*>(current + 32)) != 1)
                {
                    goto EXIT_LABEL;
                }

                auto* mapId = reinterpret_cast<std::uint16_t*>(current + 36);
                int count = 0;
                do
                {
                    token = std::strtok(nullptr, delimiter); // 17~20 wMap1Id ~ wMap4Id
                    if (!token)
                    {
                        goto EXIT_LABEL;
                    }
                    *mapId = cltInstantDungeonMapKindInfo::TranslateKindCode(token);
                    ++count;
                    ++mapId;
                } while (count < 4);

                current += sizeof(strInstantDungeonKindInfo);
                if (!std::fgets(buffer, 1023, stream))
                {
                    break;
                }
            }
        }

        result = 1;
    }

EXIT_LABEL:
    g_clTextFileManager.fclose(stream);
    return result;
}

void cltInstantDungeonKindInfo::Free()
{
    if (m_data)
    {
        ::operator delete(m_data);
        m_data = nullptr;
    }
    m_count = 0;
}

strInstantDungeonKindInfo* cltInstantDungeonKindInfo::GetInstantDungeonKindInfo(std::uint16_t kind)
{
    int index = 0;
    if (m_count <= 0)
    {
        return nullptr;
    }

    auto* base = reinterpret_cast<std::uint16_t*>(m_data);
    for (auto* it = reinterpret_cast<std::uint16_t*>(m_data); *it != kind; it += 22)
    {
        if (++index >= m_count)
        {
            return nullptr;
        }
    }

    return reinterpret_cast<strInstantDungeonKindInfo*>(&base[22 * index]);
}

strInstantDungeonKindInfo* cltInstantDungeonKindInfo::GetInstantDungeonKindInfoByItem(std::uint16_t itemKind)
{
    int index = 0;
    if (m_count <= 0)
    {
        return nullptr;
    }

    auto base = reinterpret_cast<std::uintptr_t>(m_data);
    for (auto* it = reinterpret_cast<std::uint16_t*>(base + 8); *it != itemKind; it += 22)
    {
        if (++index >= m_count)
        {
            return nullptr;
        }
    }

    return reinterpret_cast<strInstantDungeonKindInfo*>(base + sizeof(strInstantDungeonKindInfo) * index);
}

std::uint16_t cltInstantDungeonKindInfo::GetInstantDungeonKindByItem(std::uint16_t itemKind)
{
    int index = 0;
    if (m_count <= 0)
    {
        return 0;
    }

    auto base = reinterpret_cast<std::uintptr_t>(m_data);
    for (auto* it = reinterpret_cast<std::uint16_t*>(base + 8); *it != itemKind; it += 22)
    {
        if (++index >= m_count)
        {
            return 0;
        }
    }

    return *reinterpret_cast<std::uint16_t*>(base + sizeof(strInstantDungeonKindInfo) * index);
}

std::uint16_t cltInstantDungeonKindInfo::TranslateKindCode(char* text)
{
    char buffer[256];

    if (std::strlen(text) != 5)
    {
        return 0;
    }

    const int prefix = (std::toupper(*text) + 31) << 11;
    const std::uint16_t value = static_cast<std::uint16_t>(std::atoi(text + 1));
    if (value < 0x800u)
    {
        return static_cast<std::uint16_t>(prefix | value);
    }

    std::memset(buffer, 0, sizeof(buffer));
    std::sprintf(buffer, "Numberic 2048 Over (%s)", text);
    MessageBoxA(nullptr, "InstantDungeonKindInfo Error", "Error", MB_OK);
    MessageBoxA(nullptr, buffer, "Error", MB_OK);
    return 0;
}

strInstantDungeonKindInfo* cltInstantDungeonKindInfo::GetInstantDungeonKindInfoByIndex(int index)
{
    // 依反編譯碼保留原始邏輯：index == m_count 時仍會回傳越界指標。
    if (index < 0 || index > m_count)
    {
        return nullptr;
    }

    return reinterpret_cast<strInstantDungeonKindInfo*>(
        reinterpret_cast<std::uintptr_t>(m_data) + sizeof(strInstantDungeonKindInfo) * index);
}
