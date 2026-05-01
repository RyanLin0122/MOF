#include "Info/cltPetKindInfo.h"
#include "System/cltPetMarketMySalePetSystem.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <new>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
    constexpr std::size_t kLineBufferSize = 1024;

    // 1:1 對應 mofclient.c 全域 ::IsDigit (sub_0059FE90)。命名為 PetKindIsDigit
    // 以避免與 cltItemKindInfo.h 中已存在但語意不同 (拒空字串/拒符號) 的 inline ::IsDigit 衝突。
    // 行為:
    //   - 空字串 → 1 (true)
    //   - 任意位置出現 '+' 或 '-' 會被「跳過一次」(原始反編譯邏輯，即使位於中段也接受)
    //   - 跳過的字元若不是 0..9 則回 0
    //   - 走到結尾則回 1
    int PetKindIsDigit(const char* text)
    {
        if (text == nullptr || *text == '\0')
        {
            return 1;
        }

        const unsigned char* p = reinterpret_cast<const unsigned char*>(text);
        while (true)
        {
            if (*p == '-' || *p == '+')
            {
                ++p;
            }
            if (!std::isdigit(*p))
            {
                return 0;
            }
            ++p;
            if (*p == '\0')
            {
                return 1;
            }
        }
    }

    void ShowPetKindFormatWarning(const char* filePath)
    {
        char text[1024] = {};
        std::snprintf(text, sizeof(text), "LoadPetKindInfo warning: optional last column is missing in file: %s", filePath ? filePath : "<null>");

#ifdef _WIN32
        ::MessageBoxA(nullptr, text, "Error", 0);
#else
        std::fprintf(stderr, "%s\n", text);
#endif
    }

    uint8_t TranslatePetCategory(const char* text)
    {
        if (std::strcmp(text, "BASICPET") == 0)   return 0;
        if (std::strcmp(text, "DRAGON") == 0)     return 1;
        if (std::strcmp(text, "TINKERBELL") == 0) return 2;
        if (std::strcmp(text, "PENGUIN") == 0)    return 3;
        if (std::strcmp(text, "BEARDOLL") == 0)   return 4;
        if (std::strcmp(text, "DEVIL") == 0)      return 5;
        if (std::strcmp(text, "BISQUEDOLL") == 0) return 6;
        if (std::strcmp(text, "MECHANIC") == 0)   return 7;
        return 0;
    }
} // namespace

cltPetKindInfo::cltPetKindInfo()
    : m_pPetKindInfo(nullptr)
    , m_nPetKindInfoCount(0)
    , m_pPetDyeKindInfo(nullptr)
    , m_nPetDyeKindInfoCount(0)
{
}

cltPetKindInfo::~cltPetKindInfo()
{
    Free();
}

int cltPetKindInfo::Initialize(char* petKindPath, char* petDyeKindPath)
{
    int result = LoadPetKindInfo(petKindPath);
    if (result)
    {
        result = (LoadPetDyeKindInfo(petDyeKindPath) != 0) ? 1 : 0;
    }
    return result;
}

int cltPetKindInfo::LoadPetKindInfo(char* filePath)
{
    char delimiter[3] = "\t\n";
    char buffer[kLineBufferSize] = {};
    int result = 0;
    int strictFormatOk = 1;

    std::FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr)
    {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream))
    {
        fpos_t position{};
        std::fgetpos(stream, &position);

        for (; std::fgets(buffer, 1023, stream); ++m_nPetKindInfoCount)
        {
        }

        m_pPetKindInfo = static_cast<strPetKindInfo*>(::operator new(sizeof(strPetKindInfo) * static_cast<std::size_t>(m_nPetKindInfoCount)));
        std::memset(m_pPetKindInfo, 0, sizeof(strPetKindInfo) * static_cast<std::size_t>(m_nPetKindInfoCount));

        std::fsetpos(stream, &position);

        strPetKindInfo* current = m_pPetKindInfo;
        if (std::fgets(buffer, 1023, stream))
        {
            while (true)
            {
                char* token = std::strtok(buffer, delimiter);
                if (token == nullptr)
                {
                    break;
                }

                current->wPetId = TranslateKindCode(token);
                if (current->wPetId == 0)
                {
                    break;
                }

                if (std::strtok(nullptr, delimiter) == nullptr) // skip 1 column
                {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // wPetNameTextCode
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wPetNameTextCode = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wPetDescriptionTextCode
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wPetDescriptionTextCode = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wPreviousPetIdOnJobChange
                if (token == nullptr)
                {
                    break;
                }
                current->wPreviousPetIdOnJobChange = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // AnimationInfoFileGi
                if (token == nullptr)
                {
                    break;
                }
                if (std::strstr(token, ".txt") == nullptr)
                {
                    break;
                }
                std::strcpy(current->AnimationInfoFileGi, token);

                token = std::strtok(nullptr, delimiter); // wDyeCount
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wDyeCount = static_cast<uint16_t>(std::atoi(token));

                for (int i = 0; i < current->wDyeCount; ++i)
                {
                    token = std::strtok(nullptr, delimiter);
                    if (token == nullptr)
                    {
                        goto parse_done;
                    }
                    current->awDyeCodes[i] = TranslateKindCode(token);
                }

                token = std::strtok(nullptr, delimiter); // wCharacterWidth
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wCharacterWidth = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wCharacterHeightPosY
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wCharacterHeightPosY = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwDotResourceId
                if (token == nullptr)
                {
                    break;
                }
                std::sscanf(token, "%x", &current->dwDotResourceId);

                token = std::strtok(nullptr, delimiter); // wBlockId
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wBlockId = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwPetUiAlertResource
                if (token == nullptr)
                {
                    break;
                }
                std::sscanf(token, "%x", &current->dwPetUiAlertResource);

                token = std::strtok(nullptr, delimiter); // wBlockIdDuplicate
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wBlockIdDuplicate = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wBasicPassiveSkill
                if (token == nullptr)
                {
                    break;
                }
                current->wBasicPassiveSkill = cltPetSkillKindInfo::TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // bPetCategory (string)
                if (token == nullptr)
                {
                    break;
                }
                current->bPetCategory = TranslatePetCategory(token);

                if (std::strtok(nullptr, delimiter) == nullptr) // skip 1 column
                {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // dwLoveExperience
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->dwLoveExperience = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wSaturation
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wSaturation = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwSaturationDecreasePerMinute
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->dwSaturationDecreasePerMinute = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wPetStageBaseYCoordinate
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wPetStageBaseYCoordinate = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wPetTypeTextCode
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wPetTypeTextCode = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wPetBasicSkillNameTextCode
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wPetBasicSkillNameTextCode = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwSkillAcquisitionStatus
                if (token == nullptr)
                {
                    break;
                }
                current->dwSkillAcquisitionStatus = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // bDefaultBagCount
                if (token == nullptr)
                {
                    break;
                }
                current->bDefaultBagCount = static_cast<uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // bMaxExtendedBagCount
                if (token == nullptr)
                {
                    break;
                }
                current->bMaxExtendedBagCount = static_cast<uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwPetNameChangePossible
                if (token == nullptr)
                {
                    break;
                }
                current->dwPetNameChangePossible = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // bCharacterFrontBack
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->bCharacterFrontBack = static_cast<uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwPetPositionAdjustmentY
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->dwPetPositionAdjustmentY = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwPetRadiationPriceLib
                if (token == nullptr)
                {
                    break;
                }
                current->dwPetRadiationPriceLib = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwRadiationSkillUnitPriceLib
                if (token == nullptr)
                {
                    break;
                }
                current->dwRadiationSkillUnitPriceLib = static_cast<uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // dwPetStopResource (hex)
                if (token == nullptr)
                {
                    break;
                }
                std::sscanf(token, "%x", &current->dwPetStopResource);

                token = std::strtok(nullptr, delimiter); // wPetStopBlockId
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->wPetStopBlockId = static_cast<uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // wPetEffect
                if (token == nullptr)
                {
                    break;
                }
                current->wPetEffect = TranslateKindCode(token);

                current->wPetLevel = static_cast<uint16_t>(GetPetLevel(current->wPetId));

                token = std::strtok(nullptr, delimiter); // wPetRequiredLevel (optional)
                if (token != nullptr)
                {
                    current->wPetRequiredLevel = static_cast<uint16_t>(std::atoi(token));
                }
                else
                {
                    strictFormatOk = 0;
                }

                ++current;
                if (std::fgets(buffer, 1023, stream) == nullptr)
                {
                    result = 1;
                    goto parse_done;
                }
            }
        }
        else
        {
            result = 1;
        }

    parse_done:
        if (!strictFormatOk)
        {
            ShowPetKindFormatWarning(filePath);
        }
    }

    g_clTextFileManager.fclose(stream);
    return result;
}

int cltPetKindInfo::LoadPetDyeKindInfo(char* filePath)
{
    char delimiter[3] = "\t\n";
    char buffer[kLineBufferSize] = {};
    int result = 0;

    std::FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr)
    {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream))
    {
        fpos_t position{};
        std::fgetpos(stream, &position);

        for (; std::fgets(buffer, 1023, stream); ++m_nPetDyeKindInfoCount)
        {
        }

        m_pPetDyeKindInfo = static_cast<strPetDyeKindInfo*>(::operator new(sizeof(strPetDyeKindInfo) * static_cast<std::size_t>(m_nPetDyeKindInfoCount)));
        std::memset(m_pPetDyeKindInfo, 0, sizeof(strPetDyeKindInfo) * static_cast<std::size_t>(m_nPetDyeKindInfoCount));

        std::fsetpos(stream, &position);

        strPetDyeKindInfo* current = m_pPetDyeKindInfo;
        if (std::fgets(buffer, 1023, stream))
        {
            while (true)
            {
                char* token = std::strtok(buffer, delimiter); // 1. 染色代碼
                if (token == nullptr)
                {
                    break;
                }

                current->wDyeCode = TranslateKindCode(token);
                if (current->wDyeCode == 0)
                {
                    break;
                }

                if (std::strtok(nullptr, delimiter) == nullptr) // 2. 顏色(企劃者用)，只驗證存在，不寫入
                {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 3. 染色索引
                if (token == nullptr || !PetKindIsDigit(token))
                {
                    break;
                }
                current->dwDyeIndex = static_cast<uint32_t>(std::atoi(token));

                ++current;
                if (std::fgets(buffer, 1023, stream) == nullptr)
                {
                    result = 1;
                    goto parse_done;
                }
            }
        }
        else
        {
            result = 1;
        }
    }

parse_done:
    g_clTextFileManager.fclose(stream);
    return result;
}

void cltPetKindInfo::Free()
{
    if (m_pPetKindInfo != nullptr)
    {
        ::operator delete(m_pPetKindInfo);
        m_pPetKindInfo = nullptr;
    }

    m_nPetKindInfoCount = 0;

    if (m_pPetDyeKindInfo != nullptr)
    {
        ::operator delete(m_pPetDyeKindInfo);
        m_pPetDyeKindInfo = nullptr;
    }

    m_nPetDyeKindInfoCount = 0;
}

uint16_t cltPetKindInfo::GetNextPetKind(uint16_t previousPetId) const
{
    if (m_nPetKindInfoCount <= 0)
    {
        return 0;
    }

    for (int i = 0; i < m_nPetKindInfoCount; ++i)
    {
        if (m_pPetKindInfo[i].wPreviousPetIdOnJobChange == previousPetId)
        {
            return m_pPetKindInfo[i].wPetId;
        }
    }
    return 0;
}

strPetKindInfo* cltPetKindInfo::GetPetKindInfo(uint16_t petId) const
{
    if (m_nPetKindInfoCount <= 0)
    {
        return nullptr;
    }

    for (int i = 0; i < m_nPetKindInfoCount; ++i)
    {
        if (m_pPetKindInfo[i].wPetId == petId)
        {
            return &m_pPetKindInfo[i];
        }
    }
    return nullptr;
}

uint16_t cltPetKindInfo::GetOriginalPetKind(uint16_t petId) const
{
    strPetKindInfo* info = GetPetKindInfo(petId);
    return (info != nullptr) ? info->awDyeCodes[0] : 0;
}

strPetDyeKindInfo* cltPetKindInfo::GetPetDyeKindInfo(uint16_t dyeCode) const
{
    if (m_nPetDyeKindInfoCount <= 0)
    {
        return nullptr;
    }

    for (int i = 0; i < m_nPetDyeKindInfoCount; ++i)
    {
        if (m_pPetDyeKindInfo[i].wDyeCode == dyeCode)
        {
            return &m_pPetDyeKindInfo[i];
        }
    }
    return nullptr;
}

uint16_t cltPetKindInfo::GetPetTypeTextCode(uint8_t petCategory) const
{
    if (m_nPetKindInfoCount <= 0)
    {
        return 0;
    }

    for (int i = 0; i < m_nPetKindInfoCount; ++i)
    {
        if (m_pPetKindInfo[i].bPetCategory == petCategory)
        {
            return m_pPetKindInfo[i].wPetTypeTextCode;
        }
    }
    return 0;
}

uint16_t cltPetKindInfo::TranslateKindCode(const char* text)
{
    if (text == nullptr || std::strlen(text) != 5)
    {
        return 0;
    }

    const int high = (std::toupper(static_cast<unsigned char>(text[0])) + 31) << 11;
    const uint16_t low = static_cast<uint16_t>(std::atoi(text + 1));
    if (low >= 0x800u)
    {
        return 0;
    }
    return static_cast<uint16_t>(high | low);
}

int16_t cltPetKindInfo::GetPetLevel(uint16_t petId) const
{
    int16_t level = 1;
    strPetKindInfo* info = GetPetKindInfo(petId);
    if (info == nullptr)
    {
        return 0;
    }

    const uint16_t* previous = reinterpret_cast<const uint16_t*>(reinterpret_cast<const char*>(info) + 6);
    while (*previous != 0)
    {
        ++level;
        previous -= 86;
    }

    return level;
}

uint32_t cltPetKindInfo::GetPetReleaseCost(uint16_t petId, uint16_t skillCount) const
{
    strPetKindInfo* info = GetPetKindInfo(petId);
    if (info == nullptr)
    {
        return 0;
    }

    return info->dwPetRadiationPriceLib + static_cast<uint32_t>(skillCount) * info->dwRadiationSkillUnitPriceLib;
}

int cltPetKindInfo::IsSamePet(uint16_t petIdA, uint16_t petIdB) const
{
    strPetKindInfo* left = GetPetKindInfo(petIdA);
    if (left == nullptr)
    {
        return 0;
    }

    strPetKindInfo* right = GetPetKindInfo(petIdB);
    if (right == nullptr)
    {
        return 0;
    }

    if (left == right)
    {
        return 1;
    }

    while (true)
    {
        left = GetPetKindInfo(left->wPreviousPetIdOnJobChange);
        if (left == nullptr)
        {
            break;
        }
        if (left == right)
        {
            return 1;
        }
    }

    return 0;
}
