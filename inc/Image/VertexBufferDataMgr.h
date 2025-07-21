#pragma once

#include <d3d9.h>
#include <cstdio> // 用於 NULL

template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

// 根據反編譯程式碼中的用法，定義VertexBufferData結構
// 大小為 0x10 (16位元組)
struct VertexBufferData {
    VertexBufferData* pPrev;          // 位移 +0: 指向前一個節點
    VertexBufferData* pNext;          // 位移 +4: 指向後一個節點
    IDirect3DVertexBuffer9* pVertexBuffer; // 位移 +8: 指向D3D頂點緩衝區物件
    unsigned short capacity;          // 位移 +12: 緩衝區容量
    unsigned char type;               // 位移 +14: 頂點類型
    char _padding;                    // 補齊至16位元組

    // 節點的解構函式
    ~VertexBufferData() {
        // 在節點被銷毀時，自動釋放其擁有的D3D資源
        SafeRelease(pVertexBuffer);
    }
};

/**
 * @class VertexBufferDataMgr
 * @brief 管理VertexBufferData節點的雙向鏈結串列。
 *
 * 這個管理器負責建立、刪除、並在D3D裝置遺失時處理所有的頂點緩衝區資源。
 */
class VertexBufferDataMgr {
public:
    /// @brief 建構函式
    VertexBufferDataMgr();

    /// @brief 解構函式
    ~VertexBufferDataMgr();

    /// @brief 新增一個節點到鏈結串列尾部並返回。
    /// @return 指向新建立的VertexBufferData節點的指標。
    VertexBufferData* Add();

    /// @brief 從鏈結串列中刪除指定的節點。
    /// @param pNode 要刪除的節點指標。
    void Delete(VertexBufferData* pNode);

    /// @brief 刪除並釋放管理器中的所有節點和資源。
    void DeleteAll();

    /// @brief 處理D3D裝置遺失事件。
    /// 此函式會釋放所有頂點緩衝區，但保留節點結構以便後續重建。
    void DeviceLostToRelease();

    /// @brief 處理D3D裝置重設事件。
    /// 此函式會根據之前儲存的資訊，重新建立所有頂點緩衝區。
    void DeviceLostToReLoad();

private:
    VertexBufferData* m_pHead; // 鏈結串列的頭指標
    VertexBufferData* m_pTail; // 鏈結串列的尾指標
    int m_nCount;              // 鏈結串列中的節點數量
};