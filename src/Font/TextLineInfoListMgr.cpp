#include "Font/TextLineInfoListMgr.h"

// 對應反編譯碼: 0x0051E8D0
TextLineInfoListMgr::TextLineInfoListMgr() {
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// 解構函式應確保所有分配的記憶體都被釋放
TextLineInfoListMgr::~TextLineInfoListMgr() {
    DeleteAll();
}

// 對應反編譯碼: 0x0051E8F0
TLILInfo* TextLineInfoListMgr::Add() {
    // 創建一個新的節點。原始碼手動呼叫 operator new 並清零，
    // 這裡我們使用 new 並依賴 TLILInfo 的建構函式來初始化。
    TLILInfo* pNewNode = new TLILInfo();
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

// 對應反編譯碼: 0x0051E960
void TextLineInfoListMgr::DeleteAll() {
    TLILInfo* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        TLILInfo* pNodeToDelete = pCurrent;
        pCurrent = pCurrent->m_pNext;
        delete pNodeToDelete;
    }

    // 重設管理器狀態
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}
