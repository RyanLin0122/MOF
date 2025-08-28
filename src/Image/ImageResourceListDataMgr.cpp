#include "Image/ImageResourceListDataMgr.h"
#include <cstring> // 用於 strcpy

// 假設這是一個全域變數，用於決定資源的載入方式，如同在反編譯程式碼中所見
// 0: 從獨立檔案載入, 非0: 從封裝檔載入
extern int IsInMemory;

//--------------------------------------------------------------------------------
// ImageResourceListData (節點) 的實現
//--------------------------------------------------------------------------------

ImageResourceListData::ImageResourceListData() {
    pPrev = nullptr;
    pNext = nullptr;

    // m_Resource 的建構函式會被自動呼叫

    // 根據原始碼中的行為，在 Add() 函式中這些值會被初始化
    memset(m_szFileName, 0, sizeof(m_szFileName));
    m_cFlag = 0;
    m_ucPackerType = 0;
}

ImageResourceListData::~ImageResourceListData() {
    // m_Resource 的解構函式會在此處被自動呼叫，釋放其內部資源
}


//--------------------------------------------------------------------------------
// ImageResourceListDataMgr 的實現
//--------------------------------------------------------------------------------

ImageResourceListDataMgr::ImageResourceListDataMgr() {
    // 初始化鏈結串列的頭、尾指標和計數器
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

ImageResourceListDataMgr::~ImageResourceListDataMgr() {
    // 在管理器被銷毀時，確保所有已分配的節點都被釋放
    DeleteAll();
}

ImageResourceListData* ImageResourceListDataMgr::Add() {
    // 建立一個新的節點物件
    ImageResourceListData* pNewNode = new ImageResourceListData();

    // 如果鏈結串列是空的，則新節點既是頭也是尾
    if (m_pHead == nullptr) {
        m_pHead = pNewNode;
        m_pTail = pNewNode;
    }
    else {
        // 否則，將新節點附加到鏈結串列的末尾
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;
        m_pTail = pNewNode;
    }

    // 增加節點計數
    m_nCount++;

    return pNewNode;
}

void ImageResourceListDataMgr::Delete(ImageResourceListData* pNodeToDelete) {
    // 如果計數為0或指標為空，則不執行任何操作
    if (m_nCount == 0 || !pNodeToDelete) {
        return;
    }

    // 更新相鄰節點的指標
    if (pNodeToDelete->pPrev) {
        pNodeToDelete->pPrev->pNext = pNodeToDelete->pNext;
    }
    if (pNodeToDelete->pNext) {
        pNodeToDelete->pNext->pPrev = pNodeToDelete->pPrev;
    }

    // 更新頭/尾指標（如果被刪除的節點是頭或尾）
    if (m_pHead == pNodeToDelete) {
        m_pHead = pNodeToDelete->pNext;
    }
    if (m_pTail == pNodeToDelete) {
        m_pTail = pNodeToDelete->pPrev;
    }

    // 實際刪除節點並釋放記憶體
    delete pNodeToDelete;

    // 減少節點計數
    m_nCount--;

    // 如果鏈結串列變空，重設頭尾指標
    if (m_nCount == 0) {
        m_pHead = nullptr;
        m_pTail = nullptr;
    }
}

void ImageResourceListDataMgr::DeleteAll() {
    ImageResourceListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        ImageResourceListData* pNext = pCurrent->pNext;
        delete pCurrent;
        pCurrent = pNext;
    }

    // 重設管理器狀態
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

void ImageResourceListDataMgr::DeviceLostToRelease() {
    // 遍歷所有節點，呼叫其資源的 ResetGIData 方法
    for (ImageResourceListData* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        pNode->m_Resource.ResetGIData();
    }
}

void ImageResourceListDataMgr::DeviceLostToReLoad() {
    // 遍歷所有節點，根據檔案來源重新載入資源
    for (ImageResourceListData* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        if (IsInMemory) {
            // 從封裝檔載入 (packerType 儲存在 m_ucPackerType 中)
            // 注意：原始碼中 LoadGIInPack 的第三個參數 a5 未在 ImageResourceListData 中儲存
            // 這裡假設其為 0 或其他預設值。
            pNode->m_Resource.LoadGIInPack(pNode->m_szFileName, pNode->m_ucPackerType, 0);
        }
        else {
            // 從獨立檔案載入
            pNode->m_Resource.LoadGI(pNode->m_szFileName, pNode->m_ucPackerType);
        }
    }
}