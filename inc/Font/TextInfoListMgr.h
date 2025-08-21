#pragma once
#include "TILInfo.h" // 包含我們上一步還原的 TILInfo 類別

/**
 * @class TextInfoListMgr
 * @brief 管理 TILInfo 節點的雙向鏈結串列，作為頂層的文字快取容器。
 *
 * 這個類別從 MOFFont.cpp (constructor at 0x0051E9A0) 的反編譯程式碼中還原。
 * 它是 MoFFont 類別的成員，負責維護所有已渲染字串的快取列表，
 * MoFFont 透過它來查找、新增和移除快取項。
 */
class TextInfoListMgr {
public:
    TextInfoListMgr();
    ~TextInfoListMgr();

    /**
     * @brief 在鏈結串列的尾部新增一個新的 TILInfo 節點。
     * @return 指向新創建節點的指標。
     */
    TILInfo* Add();

    /**
     * @brief 從鏈結串列中移除並刪除指定的節點。
     * @param pNodeToDelete 要刪除的節點指標。
     */
    void Delete(TILInfo* pNodeToDelete);

    /**
     * @brief 刪除並釋放鏈結串列中的所有節點。
     */
    void DeleteAll();

    TILInfo* m_pHead;    // 鏈結串列的頭部指標 (位移: +0)
    TILInfo* m_pTail;    // 鏈結串列的尾部指標 (位移: +4)
    int      m_nCount;   // 鏈結串列中的節點數量 (位移: +8)
};
