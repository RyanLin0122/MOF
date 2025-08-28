#include "Image/cltBaseResource.h"
#include <cstring> // 使用 memmove, memset

// 對應反編譯碼: 0x005442C0
cltBaseResource::cltBaseResource() {
    // 初始化所有成員變數
    m_pResourceArray = nullptr;
    m_uResourceArrayCapacity = 0;
    m_uResourceCount = 0;
    m_dwTimeout = 0;
    m_bInitialized = false; // 對應 *((_BYTE *)this + 20) = 0;
}

// 對應反編譯碼: 0x005442F0
cltBaseResource::~cltBaseResource() {
    Free();
}

// 對應反編譯碼: 0x00544300
void cltBaseResource::Initialize(unsigned int capacity, unsigned int timeout) {
    m_uResourceArrayCapacity = capacity;
    m_dwTimeout = timeout;
    m_uResourceCount = 0;

    // 分配資源陣列記憶體
    m_pResourceArray = new ResourceInfo[capacity];
    // 將記憶體清零
    memset(m_pResourceArray, 0, sizeof(ResourceInfo) * capacity);
}

// 對應反編譯碼: 0x00544350
void cltBaseResource::Free() {
    if (m_pResourceArray) {
        // 先釋放所有資源
        DeleteAllResource();
        // 再釋放陣列本身
        delete[] m_pResourceArray;
        m_pResourceArray = nullptr;
    }
    m_uResourceArrayCapacity = 0;
}

// 對應反編譯碼: 0x00544380
void* cltBaseResource::Get(unsigned int id, int a3, int a4) {
    // 使用一個無限迴圈來確保在資源被新增後能成功取得
    while (true) {
        for (unsigned int i = 0; i < m_uResourceCount; ++i) {
            if (m_pResourceArray[i].id == id) {
                m_pResourceArray[i].refCount++; // 增加引用計數
                return m_pResourceArray[i].pData; // 返回資源指標
            }
        }

        // 如果找不到，則呼叫Add函式嘗試載入
        if (Add(id, a3, a4) != 3) {
            //OutputDebugStringA("Resource Add failed\n");
            return nullptr; // 新增失敗
        }
        // 如果Add成功，迴圈會繼續，並在下一次迭代中找到剛新增的資源
    }
}

// 對應反編譯碼: 0x005443F0
void* cltBaseResource::Get1(unsigned int id, int a3, unsigned char a4) {
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            m_pResourceArray[i].refCount++;
            return m_pResourceArray[i].pData;
        }
    }
    return nullptr; // 找不到直接返回
}

// 對應反編譯碼: 0x00544440
int cltBaseResource::Release(unsigned int id) {
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            m_pResourceArray[i].refCount--; // 減少引用計數
            m_pResourceArray[i].timestamp = timeGetTime(); // 更新時間戳
            return 1;
        }
    }
    return 0; // 找不到資源
}

// 對應反編譯碼: 0x00544680
int cltBaseResource::GetRefCount(unsigned int id) {
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            return m_pResourceArray[i].refCount;
        }
    }
    return 0;
}

// 對應反編譯碼: 0x00544600
void cltBaseResource::Poll() {
    if (!m_dwTimeout || m_uResourceCount == 0) {
        return; // 如果超時設為0或沒有資源，則不執行
    }

    DWORD currentTime = timeGetTime();
    // 從後往前遍歷，因為刪除操作會移動元素
    for (int i = m_uResourceCount - 1; i >= 0; --i) {
        if (m_pResourceArray[i].refCount <= 0 && (currentTime - m_pResourceArray[i].timestamp > m_dwTimeout)) {
            // 呼叫虛擬的Delete函式來移除超時的資源
            this->Delete(m_pResourceArray[i].id);
        }
    }
}

// 對應反編譯碼: 0x00544660
void cltBaseResource::DeleteAllResource() {
    // 重複刪除第一個元素，直到陣列為空
    while (m_uResourceCount > 0) {
        this->Delete(m_pResourceArray[0].id);
    }
}

// 對應反編譯碼: 0x00544490
int cltBaseResource::Add(unsigned int id, int a3, int a4) {
    // 檢查資源是否已經存在
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            return 2; // 資源已存在
        }
    }

    // 檢查陣列容量是否已滿
    if (m_uResourceCount >= m_uResourceArrayCapacity) {
        char text[256];
        wsprintfA(text, "Error! image buffer over: %0x", id);
        MessageBoxA(nullptr, text, "BaseResource::Add", MB_OK);
        return 1; // 陣列已滿
    }

    // 呼叫純虛擬函式載入資源
    // 原始碼中有個 IsInMemory 全域旗標來決定呼叫哪個載入函式
    extern int IsInMemory;
    void* pNewData = IsInMemory ?
        LoadResourceInPack(id, a3, a4) :
        LoadResource(id, a3, a4);

    if (!pNewData) {
        return 0; // 載入失敗
    }

    // 將新資源加入陣列
    ResourceInfo& newInfo = m_pResourceArray[m_uResourceCount];
    newInfo.id = id;
    newInfo.pData = pNewData;
    newInfo.refCount = 0; // 初始引用為0，Get函式會立即將其+1
    newInfo.timestamp = 0;

    m_uResourceCount++;

    return 3; // 新增成功
}

// 對應反編譯碼: 0x00544580
int cltBaseResource::Delete(unsigned int id) {
    if (!id || m_uResourceCount == 0) {
        return 0;
    }

    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            // 呼叫純虛擬函式釋放資源資料
            FreeResource(m_pResourceArray[i].pData);

            // 將陣列中後續的元素往前移動，覆蓋掉被刪除的元素
            unsigned int numToMove = m_uResourceCount - (i + 1);
            if (numToMove > 0) {
                memmove(
                    &m_pResourceArray[i],
                    &m_pResourceArray[i + 1],
                    sizeof(ResourceInfo) * numToMove
                );
            }

            m_uResourceCount--;
            // 清理移動後多出來的最後一個元素位置
            memset(&m_pResourceArray[m_uResourceCount], 0, sizeof(ResourceInfo));

            return 1; // 刪除成功
        }
    }

    return 0; // 找不到資源
}