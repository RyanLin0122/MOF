#include "Info/cltMyItemKindInfo.h"


cltMyItemKindInfo::cltMyItemKindInfo()
    : m_pInfo(nullptr), m_nCount(0)
{
}

cltMyItemKindInfo::~cltMyItemKindInfo()
{
    Free();
}

int cltMyItemKindInfo::Initialize(char* filePath)
{
    FILE* fp = g_clTextFileManager.fopen(filePath);
    FILE* stream = fp;
    int result = 0;

    char delimiter[3];
    delimiter[0] = '\t';
    delimiter[1] = '\n';
    delimiter[2] = '\0';

    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));

    if (!fp)
        return 0;

    // 必須先成功讀掉前三行
    if (std::fgets(buffer, 1023, fp) &&
        std::fgets(buffer, 1023, fp) &&
        std::fgets(buffer, 1023, fp))
    {
        fpos_t position{};
        std::fgetpos(fp, &position);

        // 從第 4 行開始計數
        for (; std::fgets(buffer, 1023, fp); ++m_nCount)
        {
        }

        m_pInfo = static_cast<strMyItemKindInfo*>(
            ::operator new(sizeof(strMyItemKindInfo) * static_cast<size_t>(m_nCount))
            );

        std::memset(m_pInfo, 0, sizeof(strMyItemKindInfo) * static_cast<size_t>(m_nCount));

        std::fsetpos(fp, &position);

        strMyItemKindInfo* cur = m_pInfo;

        // 沒有第 4 行時，直接視為成功
        if (!std::fgets(buffer, 1023, fp))
        {
            result = 1;
            g_clTextFileManager.fclose(fp);
            return result;
        }

        while (true)
        {
            char* tok = std::strtok(buffer, delimiter);   // 1 ID
            if (!tok)
                break;
            cur->wId = TranslateKindCode(tok);

            tok = std::strtok(nullptr, delimiter);        // 2 名稱(不使用)
            if (!tok)
                break;

            tok = std::strtok(nullptr, delimiter);        // 3 名稱文本代碼
            if (!tok)
                break;
            cur->wNameTextCode = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);        // 4 資源ID
            if (!tok)
                break;
            cur->ResourceId = 0;
            const int scanned = std::sscanf(tok, "%x", &cur->ResourceId);
            (void)scanned;

            tok = std::strtok(nullptr, delimiter);        // 5 區塊編號
            if (!tok)
                break;
            cur->wBlockNumber = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);        // 6 類型
            if (!tok)
                break;

            // 第一個字串常值在提供的 IDA 輸出中顯示為 `string'
            // 這裡照反編譯文字保留為 "string"
            if (STRCASECMP(tok, "string") == 0)
            {
                cur->bType = 1;
            }
            else if (STRCASECMP(tok, "LIBI") == 0)
            {
                cur->bType = 2;
            }
            else if (STRCASECMP(tok, "PSLOT") == 0)
            {
                cur->bType = 3;
            }
            else if (STRCASECMP(tok, "ITEMDROP") == 0)
            {
                cur->bType = 4;
            }
            else if (STRCASECMP(tok, "STATUS") == 0)
            {
                cur->bType = 5;
            }
            else if (STRCASECMP(tok, "FASTMOVE") == 0)
            {
                cur->bType = 6;
            }
            else if (STRCASECMP(tok, "EMBLEM") == 0)
            {
                cur->bType = 7;
            }
            else if (STRCASECMP(tok, "WAYPOINT") == 0)
            {
                cur->bType = 8;
            }
            else if (STRCASECMP(tok, "RECALL") == 0)
            {
                cur->bType = 9;
            }
            // 若不是上述任何值，原碼不會中斷，只是保持 0

            tok = std::strtok(nullptr, delimiter);        // 7 重複
            if (!tok)
                break;
            if (std::toupper(static_cast<unsigned char>(tok[0])) == 'T')
            {
                cur->dwDuplicate = 1;
            }
            else
            {
                if (std::toupper(static_cast<unsigned char>(tok[0])) != 'F')
                    break;
                cur->dwDuplicate = 0;
            }

            tok = std::strtok(nullptr, delimiter);        // 8 經驗值加成(100)
            if (!tok)
                break;
            cur->dwExpAdv100 = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);        // 9 掉落金錢數量加成(100)
            if (!tok)
                break;
            cur->dwDropMoneyAmountAdv100 = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);        // 10 是否為高級快速槽
            if (!tok)
                break;
            cur->dwIsPremiumQuickSlot = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);        // 11 配偶每日召喚充能
            if (!tok)
                break;
            cur->dwSpouseDailySummonCharge = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);        // 12 效果說明
            if (!tok)
                break;
            cur->wEffectDescription = static_cast<uint16_t>(std::atoi(tok));

            ++cur;

            if (!std::fgets(buffer, 1023, stream))
            {
                result = 1;
                g_clTextFileManager.fclose(fp);
                return result;
            }
        }
    }

    g_clTextFileManager.fclose(fp);
    return result;
}

void cltMyItemKindInfo::Free()
{
    if (m_pInfo)
    {
        ::operator delete(m_pInfo);
        m_pInfo = nullptr;
    }
    m_nCount = 0;
}

strMyItemKindInfo* cltMyItemKindInfo::GetMyItemKindInfo(uint16_t kindCode)
{
    if (m_nCount <= 0)
        return nullptr;

    for (int i = 0; i < m_nCount; ++i)
    {
        if (m_pInfo[i].wId == kindCode)
            return &m_pInfo[i];
    }

    return nullptr;
}

uint8_t cltMyItemKindInfo::GetMyItemType(uint16_t kindCode)
{
    if (m_nCount <= 0)
        return 0;

    for (int i = 0; i < m_nCount; ++i)
    {
        if (m_pInfo[i].wId == kindCode)
            return m_pInfo[i].bType;
    }

    return 0;
}

uint16_t cltMyItemKindInfo::TranslateKindCode(char* text)
{
    if (!text)
        return 0;

    if (std::strlen(text) != 5)
        return 0;

    int high = (std::toupper(static_cast<unsigned char>(text[0])) + 31) << 11;
    uint16_t low = static_cast<uint16_t>(std::atoi(text + 1));

    if (low < 0x800u)
        return static_cast<uint16_t>(high | low);

    return 0;
}