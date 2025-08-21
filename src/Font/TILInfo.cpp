#include "Font/TILInfo.h"
#include <cstring> // for strcpy_s

/**
 * @brief 建構函式
 *
 * 對應 TextInfoListMgr::Add (0x0051E9C0) 中對新節點的初始化邏輯。
 */
TILInfo::TILInfo() {
    m_pPrev = nullptr;
    m_pNext = nullptr;
    m_pszString = nullptr;
    m_szFaceName[0] = '\0';
    m_nFontWeight = 0;
    m_nFontHeight = 0;
    m_nTextWidth = 0;
    
    // 在 MoFFont::MoFFontResourceManager 中，會先檢查此旗標，再遞減引用計數。
    // 新增時，它應該是活躍的。
    m_bIsUsedThisFrame = 1; 

    // 在 MoFFont::SetTextLine 中，引用計數是從 0 開始增加的。
    m_wRefCnt = 0;
}

/**
 * @brief 解構函式
 *
 * 對應 TILInfo::~TILInfo (0x0051EAE0) 的反編譯邏輯。
 * 負責釋放所有動態分配的資源。
 */
TILInfo::~TILInfo() {
    // 1. 釋放動態分配的字串記憶體
    if (m_pszString) {
        delete[] m_pszString;
        m_pszString = nullptr;
    }

    // 2. 清空其包含的 TextLineInfoListMgr 中的所有 TLILInfo 節點
    //    m_LineMgr 的解構函式會自動呼叫 DeleteAll()，這裡無需手動操作。
    //    原始碼中手動呼叫了 DeleteAll，然後又呼叫解構函式，
    //    在現代 C++ 中，這是不必要的。RAII 機制會處理好。
}
