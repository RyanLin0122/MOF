#pragma once
#include "TLILInfo.h" // 包含我們剛才還原的 TLILInfo 結構

/**
 * @class TextLineInfoListMgr
 * @brief 管理 TLILInfo 節點的雙向鏈結串列。
 *
 * 這個類別從 MOFFont.cpp (constructor at 0x0051E8D0) 的反編譯程式碼中還原。
 * 它是 TILInfo 結構的成員，用於管理組成一行文字的多個文字片段 (TLILInfo)。
 */
class TextLineInfoListMgr {
public:
    TextLineInfoListMgr();
    ~TextLineInfoListMgr();

    /**
     * @brief 在鏈結串列的尾部新增一個新的 TLILInfo 節點。
     * @return 指向新創建節點的指標。
     */
    TLILInfo* Add();

    /**
     * @brief 刪除並釋放鏈結串列中的所有節點。
     */
    void DeleteAll();

    TLILInfo* m_pHead;    // 鏈結串列的頭部指標 (位移: +0)
    TLILInfo* m_pTail;    // 鏈結串列的尾部指標 (位移: +4)
    int       m_nCount;   // 鏈結串列中的節點數量 (位移: +8)
};
