#include "Image/VertexBufferDataMgr.h"
#include <new>
#include "Image/GIVertex.h"

// 假設的全域變數和定義，來自於原始碼上下文
extern IDirect3DDevice9* Device; // 全域D3D裝置

// VertexBufferDataMgr 的建構函式
// 對應反編譯碼: 0x00544910
VertexBufferDataMgr::VertexBufferDataMgr() {
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// VertexBufferDataMgr 的解構函式
// 對應反編譯碼: 在 Device_Reset_Manager::~Device_Reset_Manager 中被隱含呼叫
VertexBufferDataMgr::~VertexBufferDataMgr() {
    DeleteAll();
}

// 新增一個節點到鏈結串列的尾端
// 對應反編譯碼: 0x00544920
VertexBufferData* VertexBufferDataMgr::Add() {
    // 分配一個新的節點
    VertexBufferData* pNewNode = new (std::nothrow) VertexBufferData();
    if (!pNewNode) {
        return nullptr; // 記憶體分配失敗
    }

    // 初始化新節點的成員變數
    pNewNode->pPrev = nullptr;
    pNewNode->pNext = nullptr;
    pNewNode->pVertexBuffer = nullptr;
    pNewNode->capacity = 0;
    pNewNode->type = 0;

    // 將新節點加入到鏈結串列的尾部
    if (m_pTail == nullptr) {
        // 如果鏈結串列是空的
        m_pHead = pNewNode;
        m_pTail = pNewNode;
    }
    else {
        // 如果鏈結串列非空
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;
        m_pTail = pNewNode;
    }

    m_nCount++;
    return pNewNode;
}

// 從鏈結串列中刪除指定的節點
// 對應反編譯碼: 0x00544980
void VertexBufferDataMgr::Delete(VertexBufferData* pNode) {
    if (!pNode || m_nCount == 0) {
        return; // 無效操作
    }

    // 根據節點在鏈結串列中的位置更新指標
    if (pNode == m_pHead) {
        // 節點是頭節點
        m_pHead = pNode->pNext;
        if (m_pHead) {
            m_pHead->pPrev = nullptr;
        }
        else {
            // 如果刪除後鏈結串列為空，尾指標也需設為空
            m_pTail = nullptr;
        }
    }
    else if (pNode == m_pTail) {
        // 節點是尾節點
        m_pTail = pNode->pPrev;
        m_pTail->pNext = nullptr;
    }
    else {
        // 節點在中間
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }

    // 銷毀節點物件 (會自動呼叫其解構函式以釋放VB)
    delete pNode;
    m_nCount--;

    // 如果計數器為0，確保頭尾指標都為空
    if (m_nCount == 0) {
        m_pHead = nullptr;
        m_pTail = nullptr;
    }
}

// 刪除所有節點
// 對應反編譯碼: 0x00544A50
void VertexBufferDataMgr::DeleteAll() {
    VertexBufferData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        VertexBufferData* pNext = pCurrent->pNext;
        delete pCurrent; // 呼叫解構函式並釋放記憶體
        pCurrent = pNext;
    }
    // 重設管理器狀態
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// 裝置遺失時，釋放所有D3D資源
// 對應反編譯碼: 0x00544A90
void VertexBufferDataMgr::DeviceLostToRelease() {
    VertexBufferData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // 釋放VertexBuffer，但保留節點本身及其資料
        SafeRelease(pCurrent->pVertexBuffer);
        pCurrent = pCurrent->pNext;
    }
}

// 裝置重設後，重新載入D3D資源
// 對應反編譯碼: 0x00544AC0
void VertexBufferDataMgr::DeviceLostToReLoad() {
    VertexBufferData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // 如果這個節點之前有頂點緩衝區 (即使現在是空的) 且有容量
        if (pCurrent->capacity > 0) {
            // 根據儲存的類型和容量重新建立VertexBuffer
            // 這裡只還原了反編譯碼中出現的 GIVertex 相關邏輯
            switch (pCurrent->type) {
                // 根據反編譯碼，類型 0, 1, 2, 3 都會進入這個分支
            case 0:
            case 1:
            case 2:
            case 3:
                Device->CreateVertexBuffer(
                    28 * pCurrent->capacity, // 大小
                    520,                     // 用法 (D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC)
                    GIVertex::FVF,           // FVF
                    D3DPOOL_MANAGED,         // 記憶體池
                    &pCurrent->pVertexBuffer,
                    NULL
                );
                break;
            }
        }
        pCurrent = pCurrent->pNext;
    }
}