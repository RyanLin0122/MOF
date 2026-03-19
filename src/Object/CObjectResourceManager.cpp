#include "Object/CObjectResourceManager.h"
#include "Text/cltTextFileManager.h"
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <windows.h>

CObjectResourceManager::CObjectResourceManager()
    : m_nCount(0)
{
    memset(m_pObjectInfos, 0, sizeof(m_pObjectInfos));
}

CObjectResourceManager::~CObjectResourceManager()
{
    Free();
}

void CObjectResourceManager::Free()
{
    for (int i = 0; i < 0xFFFF; ++i)
    {
        if (m_pObjectInfos[i])
        {
            delete m_pObjectInfos[i];
            m_pObjectInfos[i] = nullptr;
        }
    }
}

stObjectInfo* CObjectResourceManager::GetObjectInfoByID(unsigned short wObjectID)
{
    return m_pObjectInfos[wObjectID];
}

int CObjectResourceManager::Initialize(char* szFileName)
{
    Free();

    char szBuffer[1024];
    memset(szBuffer, 0, sizeof(szBuffer));
    char szDelimiter[] = "\t\n";
    int result = 0;

    FILE* fp = g_clTextFileManager.fopen(szFileName);
    if (!fp)
    {
        MessageBoxA(0, "resource info file is not found", "ObjectResourceManager", 0);
        return 0;
    }

    // Skip 3 header lines
    fgets(szBuffer, 1023, fp);
    fgets(szBuffer, 1023, fp);
    fgets(szBuffer, 1023, fp);

    if (!fgets(szBuffer, 1023, fp))
    {
        // No data lines
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    // Parse data lines
    while (true)
    {
        // Parse object ID code (e.g. "O0001")
        char* pToken = strtok(szBuffer, szDelimiter);
        if (!pToken) break;

        unsigned short wObjectID = TranslateKindCode(pToken);

        // Allocate new stObjectInfo
        stObjectInfo* pInfo = new stObjectInfo();
        memset(pInfo, 0, sizeof(stObjectInfo));

        m_pObjectInfos[wObjectID] = pInfo;
        pInfo->m_wObjectID = wObjectID;

        // Parse resource ID (hex string like "0b000002")
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        if (strstr(pToken, "+")) break;
        sscanf(pToken, "%x", &pInfo->m_dwResourceID);

        // Parse block ID
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        unsigned int tempBlockID = 0;
        sscanf(pToken, "%u", &tempBlockID);
        pInfo->m_wBlockID = (unsigned short)tempBlockID;

        // Parse kind
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        unsigned int tempKind = 0;
        sscanf(pToken, "%u", &tempKind);
        pInfo->m_wKind = (unsigned short)tempKind;

        // Parse flip X (Y/N)
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        pInfo->m_bFlipX = (toupper(*pToken) == 'Y') ? 1 : 0;

        // Parse flip Y (Y/N)
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        pInfo->m_bFlipY = (toupper(*pToken) == 'Y') ? 1 : 0;

        // Parse scale
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        unsigned int tempScale = 0;
        sscanf(pToken, "%u", &tempScale);
        pInfo->m_wScale = (unsigned short)tempScale;

        // Parse alpha (Y/N)
        pToken = strtok(nullptr, szDelimiter);
        if (!pToken) break;
        pInfo->m_bAlpha = (toupper(*pToken) == 'Y') ? 1 : 0;

        ++m_nCount;

        if (!fgets(szBuffer, 1023, fp))
        {
            // Reached end of file successfully
            g_clTextFileManager.fclose(fp);
            return 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return result;
}

unsigned short CObjectResourceManager::TranslateKindCode(char* szCode)
{
    if (strlen(szCode) != 5)
        return 0;

    int category = (toupper(szCode[0]) + 31) << 11;
    unsigned short number = (unsigned short)atoi(szCode + 1);

    if (number >= 0x800u)
        return 0;

    return (unsigned short)(category | number);
}
