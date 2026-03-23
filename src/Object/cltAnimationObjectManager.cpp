#include "Object/cltAnimationObjectManager.h"
#include "Text/cltTextFileManager.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

// Ground truth 使用 IsAlphaNumeric() 判斷 hex 欄位合法性
// IsAlphaNumeric 檢查字串中每個字元是否都是英數字元 (isalnum)
static bool IsAlphaNumeric(const char* s)
{
    if (!*s)
        return true;
    while (*s)
    {
        if (!isalnum((unsigned char)*s))
            return false;
        ++s;
    }
    return true;
}

extern cltTextFileManager g_clTextFileManager;

cltAnimationObjectManager g_clAniObjectMgr;

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
cltAnimationObjectManager::cltAnimationObjectManager()
    : m_pAniObjectArray(nullptr)
    , m_wCount(0)
    , _pad(0)
    , m_pIndexMap(nullptr)
{
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
cltAnimationObjectManager::~cltAnimationObjectManager()
{
    if (m_pIndexMap)
    {
        delete[] m_pIndexMap;
        m_pIndexMap = nullptr;
    }
    if (m_pAniObjectArray)
    {
        delete[] m_pAniObjectArray;
        m_pAniObjectArray = nullptr;
    }
}

// -------------------------------------------------------------------------
// Initialize — 從文字檔讀取動畫物件列表
// -------------------------------------------------------------------------
int cltAnimationObjectManager::Initialize(char* filename)
{
    char buffer[1024];
    char delimiter[] = "\t\n";

    memset(buffer, 0, sizeof(buffer));

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp)
        return 0;

    // 跳過前 3 行標題
    if (!fgets(buffer, 1023, fp) ||
        !fgets(buffer, 1023, fp) ||
        !fgets(buffer, 1023, fp))
    {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄資料起始位置
    fpos_t dataPos;
    fgetpos(fp, &dataPos);

    // 第一遍：計算資料行數
    while (fgets(buffer, 1023, fp))
        ++m_wCount;

    // 配置陣列
    m_pAniObjectArray = new stAniObjectInfo[m_wCount];
    memset(m_pAniObjectArray, 0, sizeof(stAniObjectInfo) * m_wCount);
    m_pIndexMap = new std::uint16_t[m_wCount];

    // 回到資料起始位置，開始第二遍讀取
    fsetpos(fp, &dataPos);

    stAniObjectInfo* pCurrent = m_pAniObjectArray;

    if (!fgets(buffer, 1023, fp))
    {
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    for (;;)
    {
        // 跳過第一個 token (行號/序號)
        if (!strtok(buffer, delimiter))
            break;

        // KindCode
        char* token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_wKindCode = TranslateKindCode(token);

        // ResourceID
        token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_dwResourceID = (std::uint32_t)atoi(token);

        // MaxFrames
        token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_dwMaxFrames = (std::uint32_t)atoi(token);

        // HexParam — ground truth 使用 IsAlphaNumeric() 檢查合法性
        token = strtok(nullptr, delimiter);
        if (!token) break;
        if (!IsAlphaNumeric(token))
            break;
        sscanf(token, "%x", &pCurrent->m_dwHexParam);

        // Scale
        token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_wScale = (std::uint16_t)atoi(token);

        // Visible
        token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_byVisible = (std::uint8_t)atoi(token);

        // Transform
        token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_byTransform = (std::uint8_t)atoi(token);

        // Extra
        token = strtok(nullptr, delimiter);
        if (!token) break;
        pCurrent->m_byExtra = (std::uint8_t)atoi(token);

        ++pCurrent;

        if (!fgets(buffer, 1023, fp))
        {
            // 正常結束
            g_clTextFileManager.fclose(fp);
            return 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return 0;
}

// -------------------------------------------------------------------------
// GetAniObjCntInMap — 計算指定地圖上的動畫物件數量，填充索引表
// -------------------------------------------------------------------------
int cltAnimationObjectManager::GetAniObjCntInMap(std::uint16_t mapKind)
{
    memset(m_pIndexMap, 0, sizeof(std::uint16_t) * m_wCount);

    int count = 0;
    for (int i = 0; i < m_wCount; ++i)
    {
        if (mapKind == m_pAniObjectArray[i].m_wKindCode)
        {
            m_pIndexMap[count] = (std::uint16_t)i;
            ++count;
        }
    }
    return count;
}

// -------------------------------------------------------------------------
// GetAniObejctInfoByIndex — 以索引取得動畫物件資訊 (注意原始拼寫 Obejct)
// -------------------------------------------------------------------------
stAniObjectInfo* cltAnimationObjectManager::GetAniObejctInfoByIndex(std::uint16_t index)
{
    return &m_pAniObjectArray[index];
}

// -------------------------------------------------------------------------
// GetIndexInMap — 取得索引對照表中第 n 筆的索引值
// -------------------------------------------------------------------------
std::uint16_t cltAnimationObjectManager::GetIndexInMap(std::uint16_t index)
{
    return m_pIndexMap[index];
}

// -------------------------------------------------------------------------
// TranslateKindCode — 將5字元 KindCode 轉為16位元編碼
// 格式: "X0001" => (toupper(X) + 31) << 11 | atoi("0001")
// -------------------------------------------------------------------------
std::uint16_t cltAnimationObjectManager::TranslateKindCode(char* code)
{
    if (strlen(code) != 5)
        return 0;

    int high = (toupper((unsigned char)code[0]) + 31) << 11;
    std::uint16_t low = (std::uint16_t)atoi(code + 1);

    if (low >= 0x800u)
        return 0;

    return (std::uint16_t)(high | low);
}
