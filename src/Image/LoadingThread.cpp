#include "Image/LoadingThread.h"
#include "Image/ResourceMgr.h"
#include <process.h>        // 為了 _beginthreadex
#include <algorithm>      // 為了 std::find_if


LoadingThread::LoadingThread() {
    m_hThread = NULL;
    m_bIsRunning = false;
    InitializeCriticalSection(&m_cs);
}

LoadingThread::~LoadingThread() {
    // 確保執行緒已結束
    if (m_hThread) {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
    DeleteCriticalSection(&m_cs);
}

void LoadingThread::Poll() {
    EnterCriticalSection(&m_cs);
    // 如果執行緒未執行，且佇列中有任務
    if (!m_bIsRunning && !m_loadingQueue.empty()) {
        if (m_hThread) {
            CloseHandle(m_hThread); // 關閉舊的已結束的控制代碼
            m_hThread = NULL;
        }

        // 啟動新的背景執行緒
        unsigned int threadID;
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, &LoadingThread::OnBackgroundLoadUpdate, this, 0, &threadID);
        
        if (m_hThread) {
            m_bIsRunning = true;
        }
    }
    LeaveCriticalSection(&m_cs);
}

unsigned int __stdcall LoadingThread::OnBackgroundLoadUpdate(void* pParam) {
    LoadingThread* pThis = static_cast<LoadingThread*>(pParam);
    if (!pThis) return -1;

    while (true) {
        tResInfo currentRes;
        
        // 從佇列前端獲取一個任務
        EnterCriticalSection(&pThis->m_cs);
        if (pThis->m_loadingQueue.empty()) {
            LeaveCriticalSection(&pThis->m_cs);
            break; // 佇列空了，結束迴圈
        }
        currentRes = pThis->m_loadingQueue.front();
        pThis->m_loadingQueue.pop_front();
        LeaveCriticalSection(&pThis->m_cs);

        // 執行載入任務
        ResourceMgr::GetInstance()->LoadingImageResource(currentRes.dwGroupID, currentRes.dwResourceID, 1, currentRes.cPriority);
    }
    
    // 標記執行緒已結束
    pThis->ExitThread();
    return 0;
}

void LoadingThread::ExitThread() {
    EnterCriticalSection(&m_cs);
    m_bIsRunning = false;
    LeaveCriticalSection(&m_cs);
}

bool LoadingThread::AddBackGroundLoadingRes(const tResInfo& resInfo) {
    EnterCriticalSection(&m_cs);
    m_loadingQueue.push_back(resInfo);
    LeaveCriticalSection(&m_cs);
    return true;
}

bool LoadingThread::FindInResLoadingList(unsigned int resourceID) {
    EnterCriticalSection(&m_cs);
    bool found = false;
    for (const auto& item : m_loadingQueue) {
        if (item.dwResourceID == resourceID) {
            found = true;
            break;
        }
    }
    LeaveCriticalSection(&m_cs);
    return found;
}

void LoadingThread::DelResInLoadingList(unsigned int resourceID) {
    EnterCriticalSection(&m_cs);
    m_loadingQueue.erase(
        std::remove_if(m_loadingQueue.begin(), m_loadingQueue.end(), 
            [resourceID](const tResInfo& item) {
                return item.dwResourceID == resourceID;
            }), 
        m_loadingQueue.end());
    LeaveCriticalSection(&m_cs);
}

void LoadingThread::ClearLodingList() {
    EnterCriticalSection(&m_cs);
    m_loadingQueue.clear();
    LeaveCriticalSection(&m_cs);
}