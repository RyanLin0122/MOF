#include <cctype>
#include <cstdlib>
#include <cstring>
#include "Info/cltPetSkillKindInfo.h"
#include "global.h"


namespace
{
    bool IsDigit(const char* text)
    {
        if (text == nullptr || *text == '\0')
        {
            return false;
        }

        for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p != '\0'; ++p)
        {
            if (!std::isdigit(*p))
            {
                return false;
            }
        }
        return true;
    }
}

cltPetSkillKindInfo::cltPetSkillKindInfo()
    : m_pInfo(nullptr)
    , m_nCount(0)
{
}

void cltPetSkillKindInfo::Free()
{
    if (m_pInfo != nullptr)
    {
        ::operator delete(m_pInfo);
        m_pInfo = nullptr;
    }

    m_nCount = 0;
}

int cltPetSkillKindInfo::Initialize(char* fileName)
{
    char buffer[1024] = {};
    char delimiter[] = "\t\n";
    std::fpos_t position{};
    int result = 0;

    std::FILE* stream = g_clTextFileManager.fopen(fileName);
    std::FILE* file = stream;
    if (file == nullptr)
    {
        return 0;
    }

    if (std::fgets(buffer, 1023, file) != nullptr &&
        std::fgets(buffer, 1023, file) != nullptr &&
        std::fgets(buffer, 1023, file) != nullptr)
    {
        std::fgetpos(file, &position);
        for (; std::fgets(buffer, 1023, file) != nullptr; ++m_nCount)
        {
        }

        m_pInfo = static_cast<strPetSkillKindInfo*>(
            ::operator new(sizeof(strPetSkillKindInfo) * static_cast<std::size_t>(m_nCount)));
        std::memset(m_pInfo, 0, sizeof(strPetSkillKindInfo) * static_cast<std::size_t>(m_nCount));

        std::fsetpos(file, &position);

        auto* cursor = reinterpret_cast<std::uint8_t*>(m_pInfo);
        if (std::fgets(buffer, 1023, file) == nullptr)
        {
            result = 1;
            g_clTextFileManager.fclose(file);
            return result;
        }

        while (true)
        {
            auto* entry = reinterpret_cast<strPetSkillKindInfo*>(cursor);

            char* token = std::strtok(buffer, delimiter); // 1. 技能ID
            if (token == nullptr)
            {
                break;
            }

            const std::uint16_t skillId = TranslateKindCode(token);
            entry->wSkillId = skillId;
            if (skillId == 0)
            {
                break;
            }

            if (std::strtok(nullptr, delimiter) == nullptr) // 2. 技能名稱(企劃者用)，只跳過不存
            {
                break;
            }

            token = std::strtok(nullptr, delimiter); // 3. 技能名稱文字ID
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->wSkillNameTextId = static_cast<std::uint16_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 4. 技能說明文字ID
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->wSkillDescriptionTextId = static_cast<std::uint16_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 5. 圖示檔案名稱
            if (token == nullptr)
            {
                break;
            }
            unsigned int iconValue = 0;
            (void)std::sscanf(token, "%x", &iconValue);
            entry->IconFileName = static_cast<std::uint32_t>(iconValue);

            token = std::strtok(nullptr, delimiter); // 6. 區塊名稱
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->wBlockName = static_cast<std::uint16_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 7. 所需等級
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->wRequiredLevel = static_cast<std::uint16_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 8. 技能攻擊力上升
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwSkillAttackPowerIncrease = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 9. 攻擊力上升(千分率)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwAttackPowerIncreasePerThousand = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 10. 防禦力上升(千分率)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwDefensePowerIncreasePerThousand = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 11. 命中率上升(千分率)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwHitRateIncreasePerThousand = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 12. HP自動回復量變化率(千分率)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwHpAutoRecoveryRateChangePerThousand = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 13. MP自動回復量變化率(千分率)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwMpAutoRecoveryRateChangePerThousand = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 14. 物品掉落率上升(千分率)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwItemDropRateIncreasePerThousand = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 15. 力量(攻擊)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwStr = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 16. 體力(生命)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwVit = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 17. 敏捷(靈巧)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwDex = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 18. 智力(智慧)
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwInt = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 19. 攻擊速度
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->dwAttackSpeed = static_cast<std::uint32_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 20. 撿取
            if (token == nullptr || !IsDigit(token))
            {
                break;
            }
            entry->wPickup = static_cast<std::uint16_t>(std::atoi(token));

            token = std::strtok(nullptr, delimiter); // 21. 施法者-地面
            if (token == nullptr)
            {
                break;
            }
            entry->wCasterGround = TranslateEffectKindCode(token);

            token = std::strtok(nullptr, delimiter); // 22. 施法者-上方
            if (token == nullptr)
            {
                break;
            }
            entry->wCasterAbove = TranslateEffectKindCode(token);

            cursor += sizeof(strPetSkillKindInfo);
            if (std::fgets(buffer, 1023, file) == nullptr)
            {
                result = 1;
                g_clTextFileManager.fclose(file);
                return result;
            }
        }
    }

    g_clTextFileManager.fclose(file);
    return result;
}

strPetSkillKindInfo* cltPetSkillKindInfo::GetPetSkillKindInfo(std::uint16_t skillId)
{
    if (m_nCount <= 0)
    {
        return nullptr;
    }

    for (int i = 0; i < m_nCount; ++i)
    {
        if (m_pInfo[i].wSkillId == skillId)
        {
            return &m_pInfo[i];
        }
    }

    return nullptr;
}

std::uint16_t cltPetSkillKindInfo::TranslateKindCode(char* text)
{
    if (std::strlen(text) != 6)
    {
        return 0;
    }

    std::uint16_t high = 0;
    const int lead = std::toupper(static_cast<unsigned char>(*text));
    if (lead == 'A')
    {
        high = 0x8000;
    }
    else if (lead != 'P')
    {
        return 0;
    }

    const std::uint16_t low = static_cast<std::uint16_t>(std::atoi(text + 1));
    if (low < 0x8000u)
    {
        return static_cast<std::uint16_t>(high | low);
    }

    return 0;
}

std::uint16_t cltPetSkillKindInfo::TranslateEffectKindCode(char* text)
{
    if (std::strlen(text) != 5)
    {
        return 0;
    }

    const int high = (std::toupper(static_cast<unsigned char>(*text)) + 31) << 11;
    const std::uint16_t low = static_cast<std::uint16_t>(std::atoi(text + 1));
    if (low < 0x800u)
    {
        return static_cast<std::uint16_t>(high | low);
    }

    return 0;
}
