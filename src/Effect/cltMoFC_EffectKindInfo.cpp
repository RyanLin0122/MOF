#include "Effect/cltMoFC_EffectKindInfo.h"
#include "cltTextFileManager.h" // 假設的檔案管理器
#include <new>

// 假設的全域檔案管理器實例
extern cltTextFileManager g_clTextFileManager;

// 對應反組譯碼: 0x0053BC30
cltMoFC_EffectKindInfo::cltMoFC_EffectKindInfo()
{
    // 將指標陣列的所有成員初始化為空指標
    memset(m_pEffectInfo, 0, sizeof(m_pEffectInfo));
}

// 對應反組譯碼: 0x0053BC70 (隱含了對成員的清理)
cltMoFC_EffectKindInfo::~cltMoFC_EffectKindInfo()
{
    // 釋放所有在 Initialize 中動態分配的 stEffectKindInfo 物件
    for (int i = 0; i < 65535; ++i) {
        if (m_pEffectInfo[i]) {
            delete m_pEffectInfo[i];
            m_pEffectInfo[i] = nullptr;
        }
    }
}

// 對應反組譯碼: 0x0053BC80
int cltMoFC_EffectKindInfo::Initialize(char* szFileName)
{
    FILE* pFile = g_clTextFileManager.fopen(szFileName);
    if (!pFile) {
        return 0; // 檔案開啟失敗
    }

    char buffer[1024];
    const char* delimiters = "\t\n"; // 使用 Tab 和換行符作為分隔符

    // 跳過檔案標頭 (原始碼讀取了4次，前3次可能是註解或標題行)
    for (int i = 0; i < 4; ++i) {
        if (!fgets(buffer, sizeof(buffer), pFile)) {
            g_clTextFileManager.fclose(pFile);
            return 1; // 檔案內容不完整，但視為正常結束
        }
    }

    // 開始逐行解析
    do {
        char* token = strtok(buffer, delimiters);
        if (!token) continue; // 空行

        unsigned short kindID = TranslateKindCode(token);
        if (kindID == 0) continue; // 無效的 Kind Code

        // 如果該 ID 尚未被定義
        if (m_pEffectInfo[kindID] == nullptr) {
            stEffectKindInfo* pNewInfo = new (std::nothrow) stEffectKindInfo();
            if (!pNewInfo) break; // 記憶體分配失敗

            m_pEffectInfo[kindID] = pNewInfo;
            pNewInfo->usKindID = kindID;

            // 解析檔案名稱 (跳過第二個欄位)
            strtok(nullptr, delimiters);
            token = strtok(nullptr, delimiters);
            if (token) {
                strcpy_s(pNewInfo->szFileName, sizeof(pNewInfo->szFileName), token);
            }

            // 解析特效類型
            token = strtok(nullptr, delimiters);
            if (token) {
                if (_stricmp(token, "ONCE") == 0) pNewInfo->ucType = EFFECT_TYPE_ONCE;
                else if (_stricmp(token, "DIRECT") == 0) pNewInfo->ucType = EFFECT_TYPE_DIRECTED;
                else if (_stricmp(token, "SUSTAIN") == 0) pNewInfo->ucType = EFFECT_TYPE_SUSTAIN;
                else if (_stricmp(token, "SHOOTUNIT") == 0) pNewInfo->ucType = EFFECT_TYPE_SHOOT_UNIT;
                else if (_stricmp(token, "SHOOTNOTEA") == 0) pNewInfo->ucType = EFFECT_TYPE_SHOOT_NO_EA;
                else if (_stricmp(token, "ITEMONCE") == 0) pNewInfo->ucType = EFFECT_TYPE_ITEM_ONCE;
                else pNewInfo->ucType = EFFECT_TYPE_UNKNOWN;
            }
        }
    } while (fgets(buffer, sizeof(buffer), pFile));

    g_clTextFileManager.fclose(pFile);
    return 1; // 初始化成功
}

// 對應反組譯碼: 0x0053BF20
stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(unsigned short kindID)
{
    // 直接使用 ID 作為索引查詢陣列
    if (kindID > 0 && kindID < 65535) {
        return m_pEffectInfo[kindID];
    }
    return nullptr;
}

// 對應反組譯碼: 0x0053BF30
stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(char* szKindCode)
{
    unsigned short kindID = TranslateKindCode(szKindCode);
    return GetEffectKindInfo(kindID);
}

// 對應反組譯碼: 0x0053BF50
unsigned short cltMoFC_EffectKindInfo::TranslateKindCode(char* szKindCode)
{
    if (strlen(szKindCode) != 5) {
        return 0; // 格式不符
    }

    // 將第一個字元 ('A'~'P') 轉換為高位元
    int high_part = (toupper(szKindCode[0]) + 31) << 11;

    // 將後四位數字轉換為低位元
    int low_part = atoi(szKindCode + 1);

    // 檢查數字部分是否在有效範圍內 (0-2047)
    if (low_part < 0x800) { // 0x800 = 2048
        return static_cast<unsigned short>(high_part | low_part);
    }

    return 0; // 超出範圍
}