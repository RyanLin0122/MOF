#pragma once
#include "TextLineInfoListMgr.h" // 包含我們上一步還原的管理器
#include <windows.h>              // 包含 WORD, BYTE 等 Windows 資料類型

/**
 * @class TILInfo
 * @brief Text Info List Info，代表一個已快取的、被完整處理過的字串。
 *
 * 這個類別從 MOFFont.cpp (size from 0x0051E9C0, destructor at 0x0051EAE0)
 * 的反編譯程式碼中還原。它是 MoFFont 效能的關鍵，儲存了從字串內容、
 * 字型屬性到最終渲染佈局的所有資訊。
 *
 * 結構大小為 72 位元組 (0x48)，與反編譯碼中的 operator new 分配大小相符。
 */
class TILInfo {
public:
    // --- 鏈結串列指標 ---
    TILInfo* m_pPrev;                       // 指向前一個節點 (位移: +0)
    TILInfo* m_pNext;                       // 指向後一個節點 (位移: +4)

    // --- 字串與字型屬性 ---
    wchar_t* m_pwszString;                  // 動態分配的字串內容 (位移: +8)
    char  m_szFaceName[32];                 // 渲染時使用的字型名稱 (位移: +12)
    int   m_nFontWeight;                    // 字型粗細 (位移: +44)

    // --- 佈局與渲染資訊 ---
    // 儲存組成此字串的所有文字片段 (TLILInfo)
    TextLineInfoListMgr m_LineMgr;          // (位移: +48, 大小: 12 bytes)
    int   m_nFontHeight;                    // 字型高度 (位移: +60)
    int   m_nTextWidth;                     // 字串渲染後的總寬度 (位移: +64)

    // --- 管理與生命週期 ---
    BYTE  m_bIsUsedThisFrame;               // 標記本幀是否被使用 (位移: +68)
    // (位移: +69) - 1位元組的記憶體對齊填充 (padding)
    WORD  m_wRefCnt;                        // 引用計數，用於快取銷毀 (位移: +70)

public:
    TILInfo();
    ~TILInfo();
};
