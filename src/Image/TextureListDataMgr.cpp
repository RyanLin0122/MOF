#include "Image/TextureListDataMgr.h"
#include <cstring> // 用於 memset
#include <new>

// 假設的全域變數，來自於原始碼上下文
extern IDirect3DDevice9* Device; // 全域D3D裝置

template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

// TextureListDataMgr 的建構函式
// 對應反編譯碼: 0x00544710
TextureListDataMgr::TextureListDataMgr() {
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// TextureListDataMgr 的解構函式
// 對應反編譯碼: 在 Device_Reset_Manager::~Device_Reset_Manager 中被隱含呼叫
TextureListDataMgr::~TextureListDataMgr() {
    DeleteAll();
}

// 新增一個節點到鏈結串列的尾端
// 對應反編譯碼: 0x00544720
TextureListData* TextureListDataMgr::Add() {
    // 分配一個新的節點
    TextureListData* pNewNode = new (std::nothrow) TextureListData();
    if (!pNewNode) {
        return nullptr; // 記憶體分配失敗
    }

    // 初始化新節點的成員變數
    pNewNode->pPrev = nullptr;
    pNewNode->pNext = nullptr;
    pNewNode->pTexture = nullptr;
    std::memset(pNewNode->szFileName, 0, sizeof(pNewNode->szFileName));
    pNewNode->flag = 0;

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
// 對應反編譯碼: 0x00544780
void TextureListDataMgr::Delete(TextureListData* pNode) {
    if (!pNode || m_nCount == 0) {
        return; // 無效操作
    }

    // 根據節點在鏈結串列中的位置更新指標
    if (pNode == m_pHead) {
        m_pHead = pNode->pNext;
        if (m_pHead) {
            m_pHead->pPrev = nullptr;
        }
        else {
            m_pTail = nullptr;
        }
    }
    else if (pNode == m_pTail) {
        m_pTail = pNode->pPrev;
        m_pTail->pNext = nullptr;
    }
    else {
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }

    // 銷毀節點物件 (會自動呼叫其解構函式以釋放Texture)
    delete pNode;
    m_nCount--;

    // 如果計數器為0，確保頭尾指標都為空
    if (m_nCount == 0) {
        m_pHead = nullptr;
        m_pTail = nullptr;
    }
}

// 刪除所有節點
// 對應反編譯碼: 0x00544850
void TextureListDataMgr::DeleteAll() {
    TextureListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        TextureListData* pNext = pCurrent->pNext;
        delete pCurrent; // 呼叫解構函式並釋放記憶體
        pCurrent = pNext;
    }
    // 重設管理器狀態
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// 裝置遺失時，釋放所有D3D資源
// 對應反編譯碼: 0x00544890
void TextureListDataMgr::DeviceLostToRelease() {
    TextureListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // 釋放Texture，但保留節點本身及其檔案名
        SafeRelease(pCurrent->pTexture);
        pCurrent = pCurrent->pNext;
    }
}

// 裝置重設後，重新載入D3D資源
// 對應反編譯碼: 0x005448C0
void TextureListDataMgr::DeviceLostToReLoad() {
    TextureListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // 如果檔案名存在，則嘗試重新載入紋理
        if (pCurrent->szFileName[0] != '\0') {
            D3DXCreateTextureFromFileExA(
                Device,
                pCurrent->szFileName,
                0xFFFFFFFF,           // Width
                0xFFFFFFFF,           // Height
                1,                    // MipLevels
                0,                    // Usage
                D3DFMT_UNKNOWN,       // Format
                D3DPOOL_MANAGED,      // Pool
                4,                    // Filter
                1,                    // MipFilter
                0xFFFF00FF,           // ColorKey
                nullptr,              // SrcInfo
                nullptr,              // Palette
                &pCurrent->pTexture);
        }
        pCurrent = pCurrent->pNext;
    }
}