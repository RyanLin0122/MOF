#include "Util/cltTimer.h"

#include <new>

#ifndef __cdecl
#define __cdecl
#endif

unsigned int* cltTimer::m_pdwtimeGetTime = nullptr;
unsigned int cltTimerManager::m_dwtimeGetTime = 0;

JXList::JXList() {
    m_root.m_prev = &m_root;
    m_root.m_next = &m_root;
    m_freeRoot.m_prev = &m_freeRoot;
    m_freeRoot.m_next = &m_freeRoot;
}

void JXList::AddToFree(JXObject* obj) {
    obj->m_next = m_freeRoot.m_next;
    obj->m_prev = &m_freeRoot;
    m_freeRoot.m_next->m_prev = obj;
    m_freeRoot.m_next = obj;
    ++m_freeSize;
}

JXObject* JXList::PushBack() {
    if (m_freeRoot.m_next == &m_freeRoot) {
        return nullptr;
    }

    JXObject* obj = m_freeRoot.m_next;
    obj->m_next->m_prev = &m_freeRoot;
    m_freeRoot.m_next = obj->m_next;
    --m_freeSize;

    obj->m_prev = m_root.m_prev;
    obj->m_next = &m_root;
    m_root.m_prev->m_next = obj;
    m_root.m_prev = obj;
    ++m_size;

    return obj;
}

void JXList::Remove(JXObject* obj) {
    obj->m_prev->m_next = obj->m_next;
    obj->m_next->m_prev = obj->m_prev;
    --m_size;
    AddToFree(obj);
}

void JXList::RemoveAll() {
    while (m_root.m_next != &m_root) {
        Remove(m_root.m_next);
    }
}

void cltTimer::InitializeStaticVariable(unsigned int* a1) {
    m_pdwtimeGetTime = a1;
}

cltTimer::cltTimer() {
    m_dwTimerID = 0;
    m_dwStartTime = 0;
    m_dwEndInterval = 0;
    m_dwCustomFuncInterval = 0;
    m_pReleaseKey = nullptr;
    m_pfInit = nullptr;
    m_pfProgress = nullptr;
    m_pfTimeout = nullptr;
    m_pfCustom = nullptr;
}

cltTimer::~cltTimer() = default;

int cltTimer::Create(unsigned int a2, unsigned int a3, std::uintptr_t a4, unsigned int a5, int a6,
                     TimerCallback a7, TimerCallback a8, TimerCallback a9, TimerCallback a10, void* a11) {
    m_dwTimerID = a2;
    m_dwStartTime = *m_pdwtimeGetTime;
    m_dwEndInterval = a3;
    m_dwCustomFuncInterval = a5;
    m_dwCustomFuncArg = m_dwStartTime;
    m_bReleaseAfterTimeOut = static_cast<std::uint32_t>(a6);
    m_pfInit = a7;
    m_pfProgress = a8;
    m_pfTimeout = a9;
    m_pfCustom = a10;
    m_pReleaseKey = a11;
    m_unk4 = a4;
    Initialize();
    return 1;
}

void cltTimer::ResetTimer(unsigned int a2, std::uintptr_t a3, unsigned int a4, int a5,
                          TimerCallback a6, TimerCallback a7, TimerCallback a8, TimerCallback a9) {
    m_dwStartTime = *m_pdwtimeGetTime;
    m_dwEndInterval = a2;
    m_dwCustomFuncInterval = a4;
    m_dwCustomFuncArg = m_dwStartTime;
    m_unk4 = a3;
    m_bReleaseAfterTimeOut = static_cast<std::uint32_t>(a5);
    m_pfInit = a6;
    m_pfProgress = a7;
    m_pfTimeout = a8;
    m_pfCustom = a9;
    Initialize();
}

void cltTimer::Release() {
    m_dwTimerID = 0;
    m_pfInit = nullptr;
    m_pfProgress = nullptr;
    m_pfTimeout = nullptr;
    m_pReleaseKey = nullptr;
}

unsigned int cltTimer::GetTimerID() const { return m_dwTimerID; }

int cltTimer::IsTimeOut() const {
    unsigned int v1 = m_dwEndInterval;
    if (!v1) {
        return 0;
    }

    // 原始 GT 用無號比較；若把 v1 轉成 int，duration = 0xFFFFFFFF（表示
    // 永不 timeout）會被誤判成 -1，任何 elapsed 都「>=」它，IsTimeOut 會
    // 立刻回傳 1，導致 StartGame 建的 IncreaseLiveTime / CreateNormalSpear
    // timer 一建立就被釋放，飛矛與計時都不會動。
    unsigned int v3 = m_dwStartTime;
    unsigned int v4 = *m_pdwtimeGetTime;
    unsigned int elapsed = (v4 < v3) ? (v4 - v3 - 1) : (v4 - v3);
    return (elapsed >= v1) ? 1 : 0;
}

BOOL cltTimer::IsUsing() const { return m_dwTimerID != 0; }

void cltTimer::Initialize() {
    TimerCallback v1 = m_pfInit;
    if (v1) {
        v1(m_dwTimerID, m_unk4);
    }
}

int cltTimer::Poll() {
    if (IsTimeOut() == 1) {
        TimerCallback v2 = m_pfTimeout;
        if (v2) {
            v2(m_dwTimerID, m_unk4);
        }
        if (m_bReleaseAfterTimeOut == 1) {
            return 1;
        }
    } else {
        TimerCallback v4 = m_pfProgress;
        if (v4) {
            v4(m_dwTimerID, m_unk4);
        }

        TimerCallback v5 = m_pfCustom;
        if (v5) {
            unsigned int v6 = m_dwCustomFuncArg;
            int v7 = *m_pdwtimeGetTime;
            unsigned int v8 = *m_pdwtimeGetTime < v6 ? v7 - v6 - 1 : v7 - v6;
            if (v8 >= m_dwCustomFuncInterval) {
                int v9 = static_cast<int>(m_dwTimerID);
                m_dwCustomFuncArg = *m_pdwtimeGetTime;
                v5(v9, m_unk4);
            }
        }
    }
    return 0;
}

std::uint8_t cltTimer::GetProgressingPercent() const {
    if (IsTimeOut() != 1) {
        return static_cast<std::uint8_t>((100 * *m_pdwtimeGetTime - 100 * m_dwStartTime) / m_dwEndInterval);
    }
    return 100;
}

unsigned int cltTimer::GetTimeLeft() const {
    if (IsTimeOut() == 1) {
        return 0;
    }
    return m_dwStartTime + m_dwEndInterval - *m_pdwtimeGetTime;
}

unsigned int cltTimer::GetProgressedTime() const {
    unsigned int v1 = m_dwStartTime;
    int v2 = *m_pdwtimeGetTime;
    if (*m_pdwtimeGetTime < v1) {
        return v2 - v1 - 1;
    }
    return v2 - v1;
}

unsigned int cltTimer::GetStartTime() const { return m_dwStartTime; }

void* cltTimer::GetReleaseKey() { return m_pReleaseKey; }

unsigned int cltTimer::GetCustomFuncInterval() const { return m_dwCustomFuncInterval; }

void cltTimerManager::InitializeStaticVariable() {
    cltTimer::InitializeStaticVariable(&m_dwtimeGetTime);
    UpdateTime();
}

cltTimerManager::cltTimerManager() {
    m_pTimers = nullptr;
    m_pCurrentPollTimer = nullptr;
    m_dwMaxTimer = 0;
    m_dwtimeGetTime = 0;
    m_dwGenerateID = 1;
}

cltTimerManager::~cltTimerManager() {
    Free();
}

int cltTimerManager::Initialize(unsigned int a2) {
    if (a2 <= 1) return 0;

    m_dwMaxTimer = a2;
    m_pTimers = new (std::nothrow) cltTimer[a2];
    if (!m_pTimers) {
        m_dwMaxTimer = 0;
        return 0;
    }

    for (unsigned int i = 0; i < m_dwMaxTimer; ++i) {
        m_list.AddToFree(static_cast<JXObject*>(&m_pTimers[i]));
    }

    m_pCurrentPollTimer = nullptr;
    return 1;
}

void cltTimerManager::Free() {
    m_list.RemoveAll();
    if (m_pTimers) {
        delete[] m_pTimers;
        m_pTimers = nullptr;
    }
    m_dwMaxTimer = 0;
}

unsigned int cltTimerManager::CreateTimer(unsigned int a2, std::uintptr_t a3, unsigned int a4, int a5,
    cltTimer::TimerCallback a6, cltTimer::TimerCallback a7,
    cltTimer::TimerCallback a8, cltTimer::TimerCallback a9, void* a10) {
    JXObject* node = m_list.PushBack();
    if (!node) return 0;

    cltTimer* t = static_cast<cltTimer*>(node);

    unsigned int id = GenerateTimerID();
    t->Create(id, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    return id;
}

int cltTimerManager::ResetTimer(unsigned int a2, unsigned int a3, std::uintptr_t a4, unsigned int a5, int a6,
                                cltTimer::TimerCallback a7, cltTimer::TimerCallback a8,
                                cltTimer::TimerCallback a9, cltTimer::TimerCallback a10) {
    cltTimer* result = GetTimer(a2);
    if (result) {
        result->ResetTimer(a3, a4, a5, a6, a7, a8, a9, a10);
        return 1;
    }
    return 0;
}

void cltTimerManager::Poll() {
    cltTimer* v2 = reinterpret_cast<cltTimer*>(m_list.m_root.m_next);
    m_pCurrentPollTimer = v2;
    if (v2 == reinterpret_cast<cltTimer*>(&m_list.m_root)) {
        m_pCurrentPollTimer = nullptr;
    } else {
        do {
            unsigned int v3 = m_pCurrentPollTimer->GetTimerID();
            if (m_pCurrentPollTimer->Poll() == 1) {
                ReleaseTimer(v3);
            }
            cltTimer* v4 = reinterpret_cast<cltTimer*>(m_pCurrentPollTimer->m_next);
            m_pCurrentPollTimer = v4;
        } while (m_pCurrentPollTimer != reinterpret_cast<cltTimer*>(&m_list.m_root));
        m_pCurrentPollTimer = nullptr;
    }
}

void cltTimerManager::ReleaseTimer(unsigned int a2) {
    cltTimer* v3 = GetTimer(a2);
    JXObject* v4 = reinterpret_cast<JXObject*>(v3);
    if (v3) {
        if (v3 == m_pCurrentPollTimer) {
            m_pCurrentPollTimer = reinterpret_cast<cltTimer*>(v3->m_prev);
        }
        v3->Release();
        m_list.Remove(v4);
    }
}

void cltTimerManager::ReleaseTimersByReleaseKey(void* a2) {
    cltTimer* v5 = NULL;
    if (a2) {
        cltTimer* v3 = reinterpret_cast<cltTimer*>(m_list.m_root.m_next);
        m_pCurrentPollTimer = v3;
        if (v3 != reinterpret_cast<cltTimer*>(&m_list.m_root)) {
            do {
                unsigned int v4 = m_pCurrentPollTimer->GetTimerID();
                if (m_pCurrentPollTimer->GetReleaseKey() == a2) {
                    ReleaseTimer(v4);
                }
                v5 = reinterpret_cast<cltTimer*>(m_pCurrentPollTimer->m_next);
                m_pCurrentPollTimer = v5;
            } while (v5 != reinterpret_cast<cltTimer*>(&m_list.m_root));
        }
        m_pCurrentPollTimer = nullptr;
    }
}

void cltTimerManager::UpdateTime() { m_dwtimeGetTime = timeGetTime(); }

cltTimer* cltTimerManager::GetTimer(unsigned int a2) {
    if (!a2) {
        return nullptr;
    }

    cltTimer* v3 = reinterpret_cast<cltTimer*>(&m_list.m_root);
    int v4 = 0;
    if (!m_list.m_size) {
        return nullptr;
    }

    while (true) {
        v3 = reinterpret_cast<cltTimer*>(v3->m_next);
        if (v3->GetTimerID() == a2) {
            break;
        }
        if (static_cast<unsigned int>(++v4) >= m_list.m_size) {
            return nullptr;
        }
    }
    return v3;
}

unsigned int cltTimerManager::GenerateTimerID() {
    unsigned int result = m_dwGenerateID;
    m_dwGenerateID = result + 1;
    if (result == 0xFFFFFFFFu) {
        m_dwGenerateID = 1;
    }
    return result;
}