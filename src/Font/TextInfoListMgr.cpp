#include "Font/TextInfoListMgr.h"

// 對應反編譯碼: 0x0051E9A0
TextInfoListMgr::TextInfoListMgr() {
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// 解構函式應確保所有分配的記憶體都被釋放
TextInfoListMgr::~TextInfoListMgr() {
    DeleteAll();
}

// 對應反編譯碼: 0x0051E9C0
TILInfo* TextInfoListMgr::Add() {
    // 創建一個新的節點。TILInfo 的建構函式會初始化其內部成員。
    TILInfo* pNewNode = new TILInfo();
    if (!pNewNode) {
        return nullptr;
    }

    if (m_pHead == nullptr) {
        // 如果鏈結串列是空的
        m_pHead = pNewNode;
        m_pTail = pNewNode;
    } else {
        // 如果鏈結串列非空，則加到尾部
        m_pTail->m_pNext = pNewNode;
        pNewNode->m_pPrev = m_pTail;
        m_pTail = pNewNode;
    }

    m_nCount++;
    return pNewNode;
}

// 對應反編譯碼: 0x0051EA40
void TextInfoListMgr::Delete(TILInfo* pNodeToDelete) {
    if (!pNodeToDelete) {
        return;
    }

    // --- 從鏈結串列中移除節點 ---
    if (pNodeToDelete == m_pHead && pNodeToDelete == m_pTail) {
        // 情況1: 這是唯一的節點
        m_pHead = nullptr;
        m_pTail = nullptr;
    } else if (pNodeToDelete == m_pHead) {
        // 情況2: 這是頭部節點
        m_pHead = pNodeToDelete->m_pNext;
        if (m_pHead) {
            m_pHead->m_pPrev = nullptr;
        }
    } else if (pNodeToDelete == m_pTail) {
        // 情況3: 這是尾部節點
        m_pTail = pNodeToDelete->m_pPrev;
        if (m_pTail) {
            m_pTail->m_pNext = nullptr;
        }
    } else {
        // 情況4: 這是一個中間節點
        if (pNodeToDelete->m_pPrev) {
            pNodeToDelete->m_pPrev->m_pNext = pNodeToDelete->m_pNext;
        }
        if (pNodeToDelete->m_pNext) {
            pNodeToDelete->m_pNext->m_pPrev = pNodeToDelete->m_pPrev;
        }
    }

    // --- 釋放節點記憶體 ---
    // delete 會自動呼叫 TILInfo 的解構函式，該解構函式會處理內部資源
    delete pNodeToDelete;
    m_nCount--;
}


// 對應反編譯碼: 0x0051EB10
void TextInfoListMgr::DeleteAll() {
    TILInfo* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        TILInfo* pNodeToDelete = pCurrent;
        pCurrent = pCurrent->m_pNext;
        delete pNodeToDelete;
    }

    // 重設管理器狀態
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}
