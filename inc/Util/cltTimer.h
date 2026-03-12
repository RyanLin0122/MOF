#pragma once

#include <cstddef>
#include <cstdint>
#include <windows.h>

#ifndef __cdecl
#define __cdecl
#endif

class JXObject {
public:
    virtual ~JXObject() = default;

    std::uint32_t m_unk4 = 0;
    JXObject* m_prev = nullptr;
    JXObject* m_next = nullptr;
};

class JXList {
public:
    JXList();
    ~JXList() = default;

    void AddToFree(JXObject* obj);
    JXObject* PushBack();
    void Remove(JXObject* obj);
    void RemoveAll();

    JXObject m_root;
    std::uint32_t m_size = 0;
    JXObject m_freeRoot;
    std::uint32_t m_freeSize = 0;
};

class cltTimer : public JXObject {
public:
    using TimerCallback = void(__cdecl*)(unsigned int, unsigned int);

    static void InitializeStaticVariable(unsigned int* a1);

    cltTimer();
    virtual ~cltTimer();

    int Create(unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, int a6,
               TimerCallback a7, TimerCallback a8, TimerCallback a9, TimerCallback a10, void* a11);
    void ResetTimer(unsigned int a2, unsigned int a3, unsigned int a4, int a5,
                    TimerCallback a6, TimerCallback a7, TimerCallback a8, TimerCallback a9);
    void Release();
    unsigned int GetTimerID() const;
    int IsTimeOut() const;
    int IsUsing() const;
    void Initialize();
    int Poll();
    std::uint8_t GetProgressingPercent() const;
    unsigned int GetTimeLeft() const;
    unsigned int GetProgressedTime() const;
    unsigned int GetStartTime() const;
    void* GetReleaseKey();
    unsigned int GetCustomFuncInterval() const;

    static unsigned int* m_pdwtimeGetTime;

private:
    std::uint32_t m_dwTimerID = 0;
    std::uint32_t m_dwStartTime = 0;
    std::uint32_t m_dwEndInterval = 0;
    std::uint32_t m_dwCustomFuncInterval = 0;
    std::uint32_t m_dwCustomFuncArg = 0;
    std::uint32_t m_bReleaseAfterTimeOut = 0;
    void* m_pReleaseKey = nullptr;
    TimerCallback m_pfInit = nullptr;
    TimerCallback m_pfProgress = nullptr;
    TimerCallback m_pfTimeout = nullptr;
    TimerCallback m_pfCustom = nullptr;
};

class cltTimerManager {
public:
    static void InitializeStaticVariable();

    cltTimerManager();
    ~cltTimerManager();

    int Initialize(unsigned int a2);
    void Free();

    unsigned int CreateTimer(unsigned int a2, unsigned int a3, unsigned int a4, int a5,
        cltTimer::TimerCallback a6, cltTimer::TimerCallback a7,
        cltTimer::TimerCallback a8, cltTimer::TimerCallback a9, void* a10);
    int ResetTimer(unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, int a6,
                   cltTimer::TimerCallback a7, cltTimer::TimerCallback a8,
                   cltTimer::TimerCallback a9, cltTimer::TimerCallback a10);
    void Poll();
    void ReleaseTimer(unsigned int a2);
    void ReleaseTimersByReleaseKey(void* a2);
    static void UpdateTime();
    cltTimer* GetTimer(unsigned int a2);
    unsigned int GenerateTimerID();

    static unsigned int m_dwtimeGetTime;

private:
    cltTimer* m_pTimers = nullptr;
    std::uint32_t m_dwMaxTimer = 0;
    JXList m_list;
    std::uint32_t m_dwGenerateID = 1;
    cltTimer* m_pCurrentPollTimer = nullptr;
};

