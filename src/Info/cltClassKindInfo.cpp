#include "Info/cltClassKindInfo.h"

#include <cstring>
#include <cstdlib>
#include <cctype>

// ------------------------------------------------------------
// cltClassKindInfo
// ------------------------------------------------------------
cltClassKindInfo::cltClassKindInfo()
    : m_pInfo(nullptr)
    , m_wTotalClassNum(0)
{
}

cltClassKindInfo::~cltClassKindInfo()
{
    Free();
}

int cltClassKindInfo::Initialize(char* fileName)
{
    char delimiter[3] = "\t\n";
    int result = 0;
    std::uint64_t atb = 1;

    std::FILE* stream = g_clTextFileManager.fopen(fileName);
    if (!stream)
        return 0;

    char buffer[1024] = {};
    std::fpos_t position{};

    // 跳過前三行
    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream))
    {
        std::fgetpos(stream, &position);

        // 先算資料筆數
        for (; std::fgets(buffer, 1023, stream); ++m_wTotalClassNum)
        {
        }

        m_pInfo = new strClassKindInfo[m_wTotalClassNum];
        std::memset(m_pInfo, 0, sizeof(strClassKindInfo) * m_wTotalClassNum);

        std::fsetpos(stream, &position);

        strClassKindInfo* info = m_pInfo;

        if (std::fgets(buffer, 1023, stream))
        {
            while (true)
            {
                // 1. 職業ID
                char* tok = std::strtok(buffer, delimiter);
                if (!tok)
                    break;

                info->wClassId = !_stricmp(tok, "NONE") ? 0 : TranslateKindCode(tok);

                // 這筆資料對應的 one-hot 屬性值
                info->qwClassAtb = atb;

                // 2. 職業名稱 — 吃掉但不寫入 struct（offset 2~7 維持 0）
                if (!std::strtok(nullptr, delimiter))
                    break;

                // 3. 職業名稱代碼
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wClassNameCode = static_cast<std::uint16_t>(std::atoi(tok));

                // 4. 可轉職職業
                tok = std::strtok(nullptr, delimiter);
                if (!tok)
                    break;
                info->wTransferableClasses = TranslateKindCode(tok);

                // 5. 轉職階段
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->bTransferStage = static_cast<std::uint8_t>(std::atoi(tok));

                // 6. 最小等級
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->bMinLevel = static_cast<std::uint8_t>(std::atoi(tok));

                // 7. 目標搜尋範圍
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->dwTargetSearchRange = static_cast<std::uint32_t>(std::atoi(tok));

                // 8. 最小攻擊
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wMinAttack = static_cast<std::uint16_t>(std::atoi(tok));

                // 9. 最小敏捷
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wMinAgility = static_cast<std::uint16_t>(std::atoi(tok));

                // 10. 最小體力
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wMinHealth = static_cast<std::uint16_t>(std::atoi(tok));

                // 11. 最小智力
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wMinIntelligence = static_cast<std::uint16_t>(std::atoi(tok));

                // 12. 劍術課程熟練度
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wSwordsmanshipSkill = static_cast<std::uint16_t>(std::atoi(tok));

                // 13. 魔法課程熟練度
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wMagicSkill = static_cast<std::uint16_t>(std::atoi(tok));

                // 14. 弓術課程熟練度
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wArcherySkill = static_cast<std::uint16_t>(std::atoi(tok));

                // 15. 牧師課程熟練度
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wPriestSkill = static_cast<std::uint16_t>(std::atoi(tok));

                // 16. 盜賊課程熟練度
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wRogueSkill = static_cast<std::uint16_t>(std::atoi(tok));

                // 17. 註解代碼
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wCommentCode = static_cast<std::uint16_t>(std::atoi(tok));

                // 18. 標記圖片區塊ID
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wMarkImageBlockId = static_cast<std::uint16_t>(std::atoi(tok));

                // 19. 基本給予增益使用數
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->dwBaseBuffUsageCount = static_cast<std::uint32_t>(std::atoi(tok));

                // 20. HP(地圖自動回復量上升)
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->dwHPMapRecoveryIncrease = static_cast<std::uint32_t>(std::atoi(tok));

                // 21. 魔法(地圖自動回復量上升)
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->dwManaMapRecoveryIncrease = static_cast<std::uint32_t>(std::atoi(tok));

                // 22. 轉職時給予的物品1代碼
                tok = std::strtok(nullptr, delimiter);
                if (!tok)
                    break;
                info->wItem1CodeOnTransfer = cltItemKindInfo::TranslateKindCode(tok);

                // 23. 轉職時給予的物品1數量
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wItem1QuantityOnTransfer = static_cast<std::uint16_t>(std::atoi(tok));

                // 24. 轉職時給予的物品2代碼
                tok = std::strtok(nullptr, delimiter);
                if (!tok)
                    break;
                info->wItem2CodeOnTransfer = cltItemKindInfo::TranslateKindCode(tok);

                // 25. 轉職時給予的物品2數量
                tok = std::strtok(nullptr, delimiter);
                if (!tok || !IsDigit(tok))
                    break;
                info->wItem2QuantityOnTransfer = static_cast<std::uint16_t>(std::atoi(tok));

                ++info;
                atb <<= 1;

                if (!std::fgets(buffer, 1023, stream))
                    goto LABEL_SUCCESS;
            }
        }
        else
        {
        LABEL_SUCCESS:
            result = 1;
        }
    }

    g_clTextFileManager.fclose(stream);
    return result;
}

void cltClassKindInfo::Free()
{
    if (m_pInfo)
    {
        delete[] m_pInfo;
        m_pInfo = nullptr;
    }
    m_wTotalClassNum = 0;
}

std::uint16_t cltClassKindInfo::TranslateKindCode(char* text)
{
    if (std::strlen(text) != 3)
        return 0;

    const std::uint16_t a = static_cast<std::uint16_t>(std::toupper(static_cast<unsigned char>(text[0])) - 'A');
    const std::uint16_t b = static_cast<std::uint16_t>(std::toupper(static_cast<unsigned char>(text[1])) - 'A');
    const std::uint16_t c = static_cast<std::uint16_t>(std::toupper(static_cast<unsigned char>(text[2])) - 'A');

    return static_cast<std::uint16_t>((32 * (b | (32 * a))) | c);
}

std::uint16_t cltClassKindInfo::GetTotalClassNum()
{
    return m_wTotalClassNum;
}

strClassKindInfo* cltClassKindInfo::GetClassKindInfo(std::uint16_t classId)
{
    if (m_wTotalClassNum == 0)
        return nullptr;

    for (std::uint16_t i = 0; i < m_wTotalClassNum; ++i)
    {
        if (m_pInfo[i].wClassId == classId)
            return &m_pInfo[i];
    }
    return nullptr;
}

strClassKindInfo* cltClassKindInfo::GetClassKindInfoByIndex(unsigned int index)
{
    if (index < m_wTotalClassNum)
        return &m_pInfo[index];
    return nullptr;
}

strClassKindInfo* cltClassKindInfo::GetClassKindInfoByAtb(std::uint64_t atb)
{
    if (m_wTotalClassNum == 0)
        return nullptr;

    for (std::uint16_t i = 0; i < m_wTotalClassNum; ++i)
    {
        if (m_pInfo[i].qwClassAtb == atb)
            return &m_pInfo[i];
    }
    return nullptr;
}

int cltClassKindInfo::GetClassKindsByAtb(std::uint64_t atb, std::uint16_t* outClassIds)
{
    int count = 0;
    std::uint64_t mask = 1;

    do
    {
        if ((atb & mask) != 0)
        {
            strClassKindInfo* info = GetClassKindInfoByAtb(mask);
            if (info)
            {
                *outClassIds = info->wClassId;
                ++outClassIds;
                ++count;
            }
        }
        mask <<= 1;
    } while (mask != 0);

    return count;
}

int cltClassKindInfo::GetClassAtb(char* text)
{
    char delimiter[2] = "|";
    int low = 0;
    int high = 0;

    if (*text == '0')
        return 0;

    for (char* tok = std::strtok(text, delimiter); tok; tok = std::strtok(nullptr, delimiter))
    {
        const std::uint16_t classId = TranslateKindCode(tok);
        strClassKindInfo* info = GetClassKindInfo(classId);

        const std::uint32_t* parts = reinterpret_cast<const std::uint32_t*>(&info->qwClassAtb);
        low |= static_cast<int>(parts[0]);
        high |= static_cast<int>(parts[1]);
    }

    (void)high;
    return low;
}
