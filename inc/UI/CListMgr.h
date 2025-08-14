#pragma once

// 由於我們不知道 CUIBase 的具體實現，在此進行前向宣告
// Forward declaration for CUIBase as its full definition is not provided.
class CUIBase;

/**
 * @struct stNode
 * @brief 雙向鏈結串列的節點結構。
 *
 * 每個節點包含一個指向 CUIBase 物件的指標，以及指向上一個和下一個節點的指標。
 */
struct stNode
{
    CUIBase* pItem; // 指向節點儲存的項目
    stNode* pPrev; // 指向上一個節點
    stNode* pNext; // 指向ся下一個節點

    // 建構函式，初始化所有指標為 nullptr
    stNode() : pItem(nullptr), pPrev(nullptr), pNext(nullptr) {}
};

/**
 * @class CListMgr
 * @brief 管理 stNode 雙向鏈結串列的類別。
 *
 * 提供新增、刪除、尋找和遍歷節點等功能。
 */
class CListMgr
{
public:
    // 建構函式與解構函式
    CListMgr();
    virtual ~CListMgr();

    // 在串列頭部插入節點
    stNode* InsertHead(CUIBase* pItem);

    // 在指定節點 a2 的右側（之後）插入新節點
    stNode* InsertNodeRight(stNode* pNode, CUIBase* pItem);

    // 在指定節點 a2 的左側（之前）插入新節點
    stNode* InsertNodeLeft(stNode* pNode, CUIBase* pItem);
    
    // 在串列尾部附加新節點
    stNode* AppendNode(CUIBase* pItem);

    // 刪除指定的節點
    bool DeleteNode(stNode* pNode);

    // 透過項目指標刪除節點
    bool DeleteNodeByItem(CUIBase* pItem);

    // 透過索引尋找節點
    stNode* FindNodeByIndex(int nIndex);

    // 透過項目指標尋找節點
    stNode* FindNodeByItem(CUIBase* pItem);

    // 獲取頭部節點
    stNode* GetHead();

    // 獲取尾部節點
    stNode* GetTail();

    // 清空整個串列，釋放所有節點記憶體
    void UnInitList();

private:
    stNode* m_pHead; // 指向鏈結串列的頭部節點
};