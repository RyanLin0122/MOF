#include "Image/cltBaseResource.h"
#include <windows.h> // for timeGetTime, MessageBoxA, etc.
#include <cstdio>    // for wsprintfA
#include <string.h>  // for memmove
#include <mmsystem.h>

// --- 外部依賴的全域變數 ---
extern int IsInMemory; // 控制是從獨立檔案(0)還是封裝檔(1)載入


// --- cltBaseResource 類別的實現 ---

cltBaseResource::cltBaseResource()
{
    // 初始化所有成員變數
    m_pResourcePool = nullptr;
    m_nCapacity = 0;
    m_nItemCount = 0;
    m_nTimeout = 0;
    m_bUnkFlag = false;
}

cltBaseResource::~cltBaseResource()
{
    // 在解構時確保所有資源被釋放
    Free();
}

void cltBaseResource::Initialize(unsigned int capacity, unsigned int timeout)
{
    m_nCapacity = capacity;
    m_nTimeout = timeout;

    // 配置資源池的記憶體
    // 使用 new[] 比較符合 C++ 風格，並確保正確的解構
    m_pResourcePool = new ResourceEntry[m_nCapacity];
    memset(m_pResourcePool, 0, sizeof(ResourceEntry) * m_nCapacity);

    m_nItemCount = 0;
}

void cltBaseResource::Free()
{
    if (m_pResourcePool)
    {
        // 刪除所有還在池中的資源
        DeleteAllResource();

        // 釋放資源池本身的記憶體
        delete[] m_pResourcePool;
        m_pResourcePool = nullptr;
    }
    m_nCapacity = 0;
}

void* cltBaseResource::Get(unsigned int id, int a3, int a4)
{
    // 這是一個無限迴圈，直到資源被找到或建立失敗
    while (true)
    {
        // 1. 在現有的快取池中尋找資源
        for (unsigned int i = 0; i < m_nItemCount; ++i)
        {
            if (m_pResourcePool[i].id == id)
            {
                // 找到了，增加參考計數並返回資源指標
                m_pResourcePool[i].refCount++;
                return m_pResourcePool[i].pData;
            }
        }

        // 2. 如果沒找到，就呼叫 Add 函式嘗試載入新資源
        // Add 函式會呼叫衍生類別的虛擬載入函式 (LoadResource / LoadResourceInPack)
        int addResult = this->Add(id, a3, a4);

        // 如果 Add 失敗 (例如磁碟中無此檔案)，則跳出迴圈並返回失敗
        if (addResult != 3) // 3 代表成功加入
        {
            // OutputDebugStringA("Resource Add failed\n");
            return nullptr;
        }

        // 如果 Add 成功，while(true) 迴圈會重新開始，
        // 在下一次迭代中，資源就會在步驟 1 中被找到。
        // 這忠實地還原了反編譯程式碼的邏輯。
    }
}

int cltBaseResource::Release(unsigned int id)
{
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            // 找到了，參考計數減 1
            if (m_pResourcePool[i].refCount > 0)
            {
                m_pResourcePool[i].refCount--;
            }
            // 更新最後存取時間，用於 Poll 機制
            m_pResourcePool[i].lastAccess = timeGetTime();
            return 1; // 成功
        }
    }
    return 0; // 沒找到
}

// 這是內部函式，由 Get 呼叫
int cltBaseResource::Add(unsigned int id, int a3, int a4)
{
    // 檢查資源是否已存在 (雖然 Get 函式已經檢查過，但這是原始邏輯)
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            return 2; // 已存在
        }
    }

    // 檢查快取池是否已滿
    if (m_nItemCount >= m_nCapacity)
    {
        char szText[256];
        wsprintfA(szText, "Error! image buffer over: %0x", id);
        MessageBoxA(NULL, szText, "BaseResource::Add", 0);
        return 1; // 失敗
    }

    // 呼叫由衍生類別實現的虛擬函式來載入資源
    void* pNewResource = nullptr;
    if (IsInMemory) // 根據全域旗標決定載入方式
    {
        pNewResource = this->LoadResourceInPack(id, a3, a4);
    }
    else
    {
        pNewResource = this->LoadResource(id, a3, a4);
    }

    if (!pNewResource)
    {
        return 0; // 載入失敗
    }

    // 將新載入的資源加入到快取池中
    ResourceEntry& newEntry = m_pResourcePool[m_nItemCount];
    newEntry.id = id;
    newEntry.pData = pNewResource;
    newEntry.refCount = 0; // Get 函式之後會把它加到 1
    newEntry.lastAccess = timeGetTime();

    m_nItemCount++;

    return 3; // 成功加入
}

// 這是內部函式，由 Poll 或 DeleteAllResource 呼叫
int cltBaseResource::Delete(unsigned int id)
{
    if (!m_nItemCount || id == 0) return 0;

    int nIndex = -1;
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            nIndex = i;
            break;
        }
    }

    if (nIndex == -1) return 0; // 沒找到

    // 呼叫衍生類別的虛擬函式來釋放資源的實際資料
    this->FreeResource(m_pResourcePool[nIndex].pData);

    // 從陣列中移除該條目，並將後面的元素往前移動
    if (nIndex < (int)m_nItemCount - 1)
    {
        memmove(&m_pResourcePool[nIndex],
            &m_pResourcePool[nIndex + 1],
            sizeof(ResourceEntry) * (m_nItemCount - nIndex - 1));
    }

    m_nItemCount--;
    return 1; // 成功刪除
}

void cltBaseResource::Poll()
{
    if (!m_nTimeout || !m_nItemCount) return;

    DWORD currentTime = timeGetTime();

    // 從後往前遍歷以安全地刪除元素
    for (int i = m_nItemCount - 1; i >= 0; --i)
    {
        if (m_pResourcePool[i].refCount == 0 &&
            (currentTime - m_pResourcePool[i].lastAccess > m_nTimeout))
        {
            // 如果資源無人引用且已逾時，則刪除它
            this->Delete(m_pResourcePool[i].id);
        }
    }
}

void cltBaseResource::DeleteAllResource()
{
    // 反覆刪除第一個元素，直到快取池為空
    while (m_nItemCount > 0)
    {
        this->Delete(m_pResourcePool[0].id);
    }
}

int cltBaseResource::GetRefCount(unsigned int id)
{
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            return m_pResourcePool[i].refCount;
        }
    }
    return 0; // 沒找到
}