#include "Info/cltLessonKindInfo.h"

// ------------------------------------------------------------
// 協助函式
// ------------------------------------------------------------
static bool IsDigitString(const char* s)
{
    if (!s)
        return false;

    if (!*s)
        return true;

    while (true)
    {
        if (*s == '+' || *s == '-')
            ++s;

        if (!std::isdigit(static_cast<unsigned char>(*s)))
            return false;

        ++s;

        if (!*s)
            return true;
    }
}

// AlphaNumeric 檢查。
// 注意：這會接受 G~Z，與 "%x" 的實際可解析範圍不完全相同，
// 但這正是最貼近反編譯行為的還原。
static bool IsAlphaNumericString(const char* s)
{
    if (!s)
        return false;

    if (!*s)
        return true;

    while (*s)
    {
        if (!std::isalnum(static_cast<unsigned char>(*s)))
            return false;
        ++s;
    }
    return true;
}

// ------------------------------------------------------------
// cltLessonKindInfo
// ------------------------------------------------------------
cltLessonKindInfo::cltLessonKindInfo()
{
    // IDA: return this;
    // 真正可用的還原需初始化成安全狀態
    m_pInfo = nullptr;
    m_nCount = 0;
}

int cltLessonKindInfo::Initialize(char* fileName)
{
    FILE* stream;
    FILE* fp;
    void* mem;
    int result;
    char* token;
    char delimiter[3];
    fpos_t position;
    char buffer[1024];

    std::memset(buffer, 0, sizeof(buffer));
    std::strcpy(delimiter, "\t\n");

    result = 0;
    m_nCount = 0;

    fp = g_clTextFileManager.fopen(fileName);
    stream = fp;
    if (!fp)
        return 0;

    // 跳過前三行
    if (std::fgets(buffer, 1023, fp) &&
        std::fgets(buffer, 1023, fp) &&
        std::fgets(buffer, 1023, fp))
    {
        std::fgetpos(fp, &position);

        for (; std::fgets(buffer, 1023, fp); ++m_nCount)
        {
        }

        mem = operator new(static_cast<std::size_t>(sizeof(strLessonKindInfo) * m_nCount));
        m_pInfo = static_cast<strLessonKindInfo*>(mem);
        std::memset(m_pInfo, 0, sizeof(strLessonKindInfo) * static_cast<std::size_t>(m_nCount));

        std::fsetpos(fp, &position);

        int index = 0;

        if (std::fgets(buffer, 1023, stream))
        {
            while (true)
            {
                if (index >= m_nCount)
                    break;

                strLessonKindInfo& info = m_pInfo[index];

                // 1 課程種類
                token = std::strtok(buffer, delimiter);
                if (!token)
                    break;

                info.bClassType = static_cast<uint8_t>(GetLessonKind(token));
                if (!info.bClassType)
                    break;

                // 2 說明（讀取但不存）
                if (!std::strtok(nullptr, delimiter))
                    break;

                // 3 排名用課程種類
                token = std::strtok(nullptr, delimiter);
                if (!token)
                    break;

                info.bRankingClassType = static_cast<uint8_t>(GetLessonKindOfRanking(token));
                if (!info.bRankingClassType)
                    break;

                // 4 課程類型
                token = std::strtok(nullptr, delimiter);
                if (!token)
                    break;

                info.bLessonType = GetLessonType(token);
                if (info.bLessonType == 4)
                    break;

                // 5 名稱代碼
                token = std::strtok(nullptr, delimiter);
                if (!token || !IsDigitString(token))
                    break;

                info.wNameCode = static_cast<uint16_t>(std::atoi(token));

                // 6 說明代碼
                token = std::strtok(nullptr, delimiter);
                if (!token || !IsDigitString(token))
                    break;

                info.wDescriptionCode = static_cast<uint16_t>(std::atoi(token));

                // 7 圖示資源ID
                token = std::strtok(nullptr, delimiter);
                if (!token || !IsAlphaNumericString(token))
                    break;

                unsigned int iconValue = info.IconResourceId;
                std::sscanf(token, "%x", &iconValue);
                info.IconResourceId = static_cast<uint32_t>(iconValue);

                // 8 區塊ID
                token = std::strtok(nullptr, delimiter);
                if (!token || !IsDigitString(token))
                    break;

                info.wBlockId = static_cast<uint16_t>(std::atoi(token));

                // 9 地點代碼
                token = std::strtok(nullptr, delimiter);
                if (!token || !IsDigitString(token))
                    break;

                info.wPlaceCode = static_cast<uint16_t>(std::atoi(token));

                ++index;

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

void cltLessonKindInfo::Free()
{
    if (m_pInfo)
    {
        operator delete(m_pInfo);
        m_pInfo = nullptr;
    }
}

strLessonKindInfo* cltLessonKindInfo::GetLessonKindInfo(uint8_t classType)
{
    int i;
    int count;
    unsigned char* base;
    unsigned char* p;

    count = m_nCount;
    i = 0;
    if (count <= 0)
        return nullptr;

    base = reinterpret_cast<unsigned char*>(m_pInfo);
    for (p = reinterpret_cast<unsigned char*>(m_pInfo); *p != classType; p += 20)
    {
        if (++i >= count)
            return nullptr;
    }

    return reinterpret_cast<strLessonKindInfo*>(base + 20 * i);
}

strLessonKindInfo* cltLessonKindInfo::GetLessonKindInfoByIndex(int index)
{
    if (index < 0 || index >= m_nCount)
        return nullptr;

    return reinterpret_cast<strLessonKindInfo*>(
        reinterpret_cast<unsigned char*>(m_pInfo) + 20 * index);
}

int cltLessonKindInfo::GetLessonKind(char* text)
{
    if (!std::strcmp(text, "SWORD_1"))
        return 10;
    if (!std::strcmp(text, "SWORD_2"))
        return 11;
    if (!std::strcmp(text, "BOW_1"))
        return 20;
    if (!std::strcmp(text, "BOW_2"))
        return 21;
    if (!std::strcmp(text, "MAGIC_1"))
        return 30;
    if (!std::strcmp(text, "MAGIC_2"))
        return 31;
    if (!std::strcmp(text, "THEOLOGY_1"))
        return 40;
    if (!std::strcmp(text, "THEOLOGY_2"))
        return 41;
    return 0;
}

int cltLessonKindInfo::GetLessonKindOfRanking(char* text)
{
    if (!std::strcmp(text, "SWORD_1"))
        return 10;
    if (!std::strcmp(text, "SWORD_2"))
        return 11;
    if (!std::strcmp(text, "BOW_1"))
        return 20;
    if (!std::strcmp(text, "BOW_2"))
        return 21;
    if (!std::strcmp(text, "MAGIC_1"))
        return 30;
    if (!std::strcmp(text, "MAGIC_2"))
        return 31;
    if (!std::strcmp(text, "THEOLOGY_1"))
        return 40;
    if (!std::strcmp(text, "THEOLOGY_2"))
        return 41;
    return 0;
}

uint8_t cltLessonKindInfo::GetLessonType(char* text)
{
    if (!std::strcmp(text, "SWORD"))
        return 0;
    if (!std::strcmp(text, "BOW"))
        return 1;
    if (!std::strcmp(text, "MAGIC"))
        return 2;
    if (!std::strcmp(text, "THEOLOGY"))
        return 3;
    return 4;
}

int cltLessonKindInfo::IsValidLessonKind(uint8_t classType)
{
    int i;
    int count;
    unsigned char* p;

    count = m_nCount;
    i = 0;

    if (count <= 0)
        return 0;

    for (p = reinterpret_cast<unsigned char*>(m_pInfo); *p != classType; p += 20)
    {
        if (++i >= count)
            return 0;
    }

    return 1;
}
