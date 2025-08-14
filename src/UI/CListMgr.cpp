#include "UI/CListMgr.h"

// CUIBase 的虛設類別，僅用於編譯
class CUIBase {};

/**
 * CListMgr 建構函式
 * 初始化鏈結串列，將頭部指標設為 nullptr。
 */
CListMgr::CListMgr() : m_pHead(nullptr)
{
}

/**
 * CListMgr 解構函式
 * 確保在物件銷毀時清空串列，防止記憶體洩漏。
 */
CListMgr::~CListMgr()
{
    UnInitList();
}

/**
 * 在串列頭部插入一個新節點。
 * @param pItem 要加入串列的 CUIBase 物件指標。
 * @return 成功則返回新建立的節點指標，若項目已存在則返回 nullptr。
 */
stNode* CListMgr::InsertHead(CUIBase* pItem)
{
    // 1. 檢查項目是否已經存在於串列中，避免重複
    if (FindNodeByItem(pItem))
    {
        return nullptr;
    }

    // 2. 如果串列為空，則此操作等同於附加節點
    if (!m_pHead)
    {
        return AppendNode(pItem);
    }

    // 3. 建立一個新的節點
    stNode* pNewNode = new stNode();
    if (!pNewNode)
    {
        return nullptr; // 記憶體分配失敗
    }
    
    // 4. 設定新節點的資料
    pNewNode->pItem = pItem;
    
    // 5. 將新節點插入到串列的最前端
    pNewNode->pNext = m_pHead;  // 新節點的下一個是舊的頭部節點
    if (m_pHead)
    {
        m_pHead->pPrev = pNewNode; // 舊的頭部節點的上一個是新節點
    }
    m_pHead = pNewNode; // 更新串列的頭部指標

    return pNewNode;
}

/**
 * 在指定節點 pNode 之後插入一個新節點。
 * @param pNode 指定的節點。
 * @param pItem 要加入串列的 CUIBase 物件指標。
 * @return 成功則返回新建立的節點指標，若項目已存在則返回 nullptr。
 */
stNode* CListMgr::InsertNodeRight(stNode* pNode, CUIBase* pItem)
{
    // 1. 檢查項目是否已經存在於串列中，避免重複
    if (FindNodeByItem(pItem))
    {
        return nullptr;
    }

    // 2. 建立新節點
    stNode* pNewNode = new stNode();
    if (!pNewNode)
    {
        return nullptr; // 記憶體分配失敗
    }

    // 3. 設定新節點的資料
    pNewNode->pItem = pItem;

    // 4. 處理新節點與後一個節點的連結
    pNewNode->pNext = pNode->pNext; // 新節點的下一個是 pNode 的下一個
    if (pNode->pNext)
    {
        pNode->pNext->pPrev = pNewNode; // pNode 的下一個節點的上一個指向新節點
    }

    // 5. 處理新節點與前一個節點 (pNode) 的連結
    pNewNode->pPrev = pNode; // 新節點的上一個是 pNode
    pNode->pNext = pNewNode; // pNode 的下一個指向新節點

    return pNewNode;
}

/**
 * 在指定節點 pNode 之前插入一個新節點。
 * @param pNode 指定的節點。
 * @param pItem 要加入串列的 CUIBase 物件指標。
 * @return 成功則返回新建立的節點指標，若項目已存在或操作失敗則返回 nullptr。
 */
stNode* CListMgr::InsertNodeLeft(stNode* pNode, CUIBase* pItem)
{
    // 1. 檢查項目是否已經存在於串列中，避免重複
    if (FindNodeByItem(pItem))
    {
        return nullptr;
    }
    
    // 2. 如果 pNode 是頭部節點，此操作等同於 InsertHead
    if (pNode == m_pHead)
    {
        return InsertHead(pItem);
    }
    
    // 3. 找到 pNode 的前一個節點
    stNode* pPrevNode = pNode->pPrev;
    if (!pPrevNode)
    {
        return nullptr; // 如果沒有前一個節點且不是頭部，則鏈結串列結構有誤
    }

    // 4. 在前一個節點之後插入新節點，即為在 pNode 之前插入
    return InsertNodeRight(pPrevNode, pItem);
}

/**
 * 在串列的尾部附加一個新節點。
 * @param pItem 要加入串列的 CUIBase 物件指標。
 * @return 成功則返回新建立的節點指標，若項目已存在則返回 nullptr。
 */
stNode* CListMgr::AppendNode(CUIBase* pItem)
{
    // 1. 檢查項目是否已經存在於串列中，避免重複
    if (FindNodeByItem(pItem))
    {
        return nullptr;
    }

    // 2. 如果串列是空的，建立新節點並將其設為頭部
    if (!m_pHead)
    {
        m_pHead = new stNode();
        if (m_pHead)
        {
            m_pHead->pItem = pItem;
        }
        return m_pHead;
    }
    
    // 3. 如果串列非空，找到尾部節點
    stNode* pCurrent = m_pHead;
    while (pCurrent->pNext)
    {
        pCurrent = pCurrent->pNext;
    }
    
    // 4. 在尾部節點之後插入新節點
    return InsertNodeRight(pCurrent, pItem);
}

/**
 * 刪除指定的節點並釋放其記憶體。
 * @param pNode 要刪除的節點指標。
 * @return 成功刪除返回 true，否則返回 false。
 */
bool CListMgr::DeleteNode(stNode* pNode)
{
    // 1. 檢查指標是否有效
    if (!pNode)
    {
        return false;
    }

    // 2. 取得前後節點
    stNode* pPrevNode = pNode->pPrev;
    stNode* pNextNode = pNode->pNext;

    // 3. 處理與前一個節點的連結
    if (pPrevNode)
    {
        pPrevNode->pNext = pNextNode; // 前一個節點的下一個指向被刪除節點的下一個
    }
    else // 如果沒有前一個節點，表示 pNode 是頭部
    {
        m_pHead = pNextNode; // 更新頭部指標
    }

    // 4. 處理與後一個節點的連結
    if (pNextNode)
    {
        pNextNode->pPrev = pPrevNode; // 後一個節點的上一個指向被刪除節點的上一個
    }
    
    // 5. 釋放 pNode 的記憶體
    delete pNode;
    
    return true;
}

/**
 * 透過項目指標 pItem 尋找並刪除節點。
 * @param pItem 要刪除的節點所包含的 CUIBase 物件指標。
 * @return 成功刪除返回 true，否則返回 false。
 */
bool CListMgr::DeleteNodeByItem(CUIBase* pItem)
{
    // 1. 尋找包含該項目的節點
    stNode* pNodeToDelete = FindNodeByItem(pItem);
    
    // 2. 如果找到，則刪除它
    if (pNodeToDelete)
    {
        return DeleteNode(pNodeToDelete);
    }
    
    return false;
}

/**
 * 透過索引來尋找節點 (0-based)。
 * @param nIndex 節點的索引。
 * @return 找到則返回節點指標，否則返回 nullptr。
 */
stNode* CListMgr::FindNodeByIndex(int nIndex)
{
    stNode* pCurrent = m_pHead;
    int nCurrentIndex = 0;

    // 從頭部開始遍歷
    while (pCurrent)
    {
        // 如果當前索引符合目標索引，返回該節點
        if (nCurrentIndex == nIndex)
        {
            return pCurrent;
        }
        // 移至下一個節點並增加索引
        pCurrent = pCurrent->pNext;
        nCurrentIndex++;
    }
    
    // 遍歷完畢仍未找到，返回 nullptr
    return nullptr;
}

/**
 * 透過項目指標 pItem 尋找節點。
 * @param pItem 要尋找的 CUIBase 物件指標。
 * @return 找到則返回節點指標，否則返回 nullptr。
 */
stNode* CListMgr::FindNodeByItem(CUIBase* pItem)
{
    // 1. 檢查傳入的指標是否有效
    if (!pItem)
    {
        return nullptr;
    }
    
    stNode* pCurrent = m_pHead;
    
    // 2. 從頭部開始遍歷整個串列
    while (pCurrent)
    {
        // 3. 比較節點中的項目指標是否與目標相符
        if (pCurrent->pItem == pItem)
        {
            return pCurrent; // 找到相符的節點
        }
        pCurrent = pCurrent->pNext;
    }
    
    // 4. 遍歷完畢仍未找到
    return nullptr;
}

/**
 * 獲取鏈結串列的頭部節點。
 * @return 返回頭部節點指標。
 */
stNode* CListMgr::GetHead()
{
    return m_pHead;
}

/**
 * 獲取鏈結串列的尾部節點。
 * @return 返回尾部節點指標，若串列為空則返回 nullptr。
 */
stNode* CListMgr::GetTail()
{
    if (!m_pHead)
    {
        return nullptr;
    }

    stNode* pCurrent = m_pHead;
    while (pCurrent->pNext)
    {
        pCurrent = pCurrent->pNext;
    }
    
    return pCurrent;
}

/**
 * 清空整個鏈結串列，並釋放所有節點的記憶體。
 */
void CListMgr::UnInitList()
{
    // 重複刪除頭部節點，直到整個串列為空
    while (m_pHead)
    {
        DeleteNode(m_pHead);
    }
}
