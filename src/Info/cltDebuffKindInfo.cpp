#include "Info/cltDebuffKindInfo.h"
#include <new>


cltDebuffKindInfo::cltDebuffKindInfo()
    : m_pInfo(nullptr), m_nCount(0)
{
}

cltDebuffKindInfo::~cltDebuffKindInfo()
{
    Free();
}

int cltDebuffKindInfo::Initialize(char* fileName)
{
    char delimiter[3] = "\t\n";
    int result = 0;

    Free();

    FILE* fp = g_clTextFileManager.fopen(fileName);
    FILE* stream = fp;
    if (!fp)
        return 0;

    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));

    // 先讀三行，之後從第 4 行開始當資料區
    if (std::fgets(buffer, 1023, fp) &&
        std::fgets(buffer, 1023, fp) &&
        std::fgets(buffer, 1023, fp))
    {
        std::fpos_t position;
        std::fgetpos(fp, &position);

        // 計算資料行數
        for (; std::fgets(buffer, 1023, fp); ++m_nCount)
        {
        }

        // 用 operator new 配置 raw memory，再 memset 清零
        m_pInfo = static_cast<strDebuffKindInfo*>(::operator new(sizeof(strDebuffKindInfo) * static_cast<size_t>(m_nCount)));
        std::memset(m_pInfo, 0, sizeof(strDebuffKindInfo) * static_cast<size_t>(m_nCount));

        // 回到資料起點
        std::fsetpos(fp, &position);

        char* rowPtr = reinterpret_cast<char*>(m_pInfo);

        // 若沒有任何資料列，反編譯會直接走成功結束
        if (!std::fgets(buffer, 1023, fp))
        {
            result = 1;
            g_clTextFileManager.fclose(fp);
            return result;
        }

        while (true)
        {
            char* tok = std::strtok(buffer, delimiter); // 1. ID -> wId
            if (!tok)
                break;

            uint16_t kindCode = TranslateKindCode(tok);
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->wId = kindCode;
            if (!kindCode)
                break;

            // 2. PlannerUse：原始程式只取 token，不存入結構
            if (!std::strtok(nullptr, delimiter))
                break;

            tok = std::strtok(nullptr, delimiter); // 3. dwDuration
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDuration = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 4. dwBossDuration
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwBossDuration = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 5. dwRepeatCycle
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwRepeatCycle = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 6. dwBossRepeatCycle
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwBossRepeatCycle = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 7. dwDamageType
            if (!tok)
                break;

#if defined(_MSC_VER)
            if (_stricmp(tok, "DMGTYPE_NONE") == 0)
#else
            if (strcasecmp(tok, "DMGTYPE_NONE") == 0)
#endif
            {
                reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageType = 0;
            }
#if defined(_MSC_VER)
            else if (_stricmp(tok, "DMGTYPE_CONSTANT") == 0)
#else
            else if (strcasecmp(tok, "DMGTYPE_CONSTANT") == 0)
#endif
            {
                reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageType = 1;
            }
#if defined(_MSC_VER)
            else if (_stricmp(tok, "DMGTYPE_RATE_CHP") == 0)
#else
            else if (strcasecmp(tok, "DMGTYPE_RATE_CHP") == 0)
#endif
            {
                reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageType = 2;
            }
#if defined(_MSC_VER)
            else if (_stricmp(tok, "DMGTYPE_CALC") == 0)
#else
            else if (strcasecmp(tok, "DMGTYPE_CALC") == 0)
#endif
            {
                reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageType = 3;
            }
            else
            {
                break;
            }

            tok = std::strtok(nullptr, delimiter); // 8. dwDamageParam1
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageParam1 = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 9. dwDamageParam2
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageParam2 = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 10. dwDamageParam3
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageParam3 = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 11. dwDamageParam4
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->dwDamageParam4 = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter); // 12. wMonsterTop1
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->wMonsterTop1 = TranslateKindCode(tok);

            tok = std::strtok(nullptr, delimiter); // 13. wMonsterTop2
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->wMonsterTop2 = TranslateKindCode(tok);

            tok = std::strtok(nullptr, delimiter); // 14. wMonsterBot1
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->wMonsterBot1 = TranslateKindCode(tok);

            tok = std::strtok(nullptr, delimiter); // 15. wMonsterBot2
            if (!tok)
                break;
            reinterpret_cast<strDebuffKindInfo*>(rowPtr)->wMonsterBot2 = TranslateKindCode(tok);

            rowPtr += sizeof(strDebuffKindInfo);

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

void cltDebuffKindInfo::Free()
{
    if (m_pInfo)
    {
        ::operator delete(m_pInfo);
        m_pInfo = nullptr;
    }

    m_nCount = 0;
}

strDebuffKindInfo* cltDebuffKindInfo::GetDebuffKindInfo(uint16_t id)
{
    if (m_nCount <= 0)
        return nullptr;

    for (int i = 0; i < m_nCount; ++i)
    {
        if (m_pInfo[i].wId == id)
            return &m_pInfo[i];
    }

    return nullptr;
}

uint16_t cltDebuffKindInfo::TranslateKindCode(char* text)
{
    if (!text)
        return 0;

    if (std::strlen(text) != 5)
        return 0;

    const int upper = std::toupper(static_cast<unsigned char>(text[0]));
    const int prefix = (upper + 31) << 11;
    const int value = std::atoi(text + 1);

    if (value < 0x800)
        return static_cast<uint16_t>(prefix | value);

    return 0;
}