#include "MiniGame/PatientRecallMgr.h"

#include <cstdlib>

#include "MiniGame/CMedical.h"   // CBedstead 完整定義

// =========================================================================
// PatientRecallMgr — 對齊 mofclient.c 0x5B04F0 / 0x5B0530 / 0x5B05C0 / 0x5B0620
// =========================================================================

PatientRecallMgr::PatientRecallMgr()
    : m_beds{}
    , m_freeIndices{}
    , m_timer()
    , m_difficulty(0)
    , m_isAccumulating(0)
    , m_pad101{}
    , m_dword26(0)
{
    // mofclient.c：建構子先建 Mini_Timer，再以一個 9 次的 do/while 同步把
    // 兩條長度為 9 的 DWORD 陣列（m_beds 與 m_freeIndices）一起歸零。
}

PatientRecallMgr::~PatientRecallMgr() = default;

void PatientRecallMgr::InitPatientRecallMgr(int difficulty, CBedstead* firstBed)
{
    // mofclient.c：和建構子一樣的「9-次清零雙陣列」迴圈。
    for (int i = 0; i < 9; ++i)
    {
        m_beds[i]        = nullptr;
        m_freeIndices[i] = 0;
    }

    m_isAccumulating = 0;
    m_difficulty     = difficulty;
    m_dword26        = 0;

    // 9 張連續 CBedstead，原始為 200 bytes/張；這裡改用 C++ 陣列指標步進。
    CBedstead* bed = firstBed;
    for (int i = 0; i < 9; ++i)
    {
        m_beds[i] = bed;
        ++bed;
    }

    // mofclient.c：依難度先補上開場病人（Easy 3、Normal 5、Hard 7）。
    switch (m_difficulty)
    {
        case 0:
            for (int i = 0; i < 3; ++i)
                AutoPatientRecall();
            break;
        case 1:
            for (int i = 0; i < 5; ++i)
                AutoPatientRecall();
            break;
        case 2:
            for (int i = 0; i < 7; ++i)
                AutoPatientRecall();
            break;
        default:
            break;
    }
}

void PatientRecallMgr::AutoPatientRecall()
{
    // mofclient.c：掃描全部 9 張床，把「目前無病人」(state==0) 的床索引
    // 記到 m_freeIndices，然後從中隨機抽一張呼叫 PatientRecall。
    int freeCount = 0;
    for (int i = 0; i < 9; ++i)
    {
        CBedstead* bed = m_beds[i];
        if (bed && bed->IsPatientState() == 0)
        {
            m_freeIndices[freeCount] = i;
            ++freeCount;
        }
    }

    if (freeCount > 0)
    {
        int r = std::rand();
        int chosen = m_freeIndices[r % freeCount];
        m_beds[chosen]->PatientRecall();
    }
}

void PatientRecallMgr::Process()
{
    // mofclient.c：先計算「目前空床」數量，並用 9 - empty 推回「使用中」數量。
    int emptyCount = 0;
    for (int i = 0; i < 9; ++i)
    {
        CBedstead* bed = m_beds[i];
        if (bed && bed->IsPatientState() == 0)
            ++emptyCount;
    }
    int busyCount = 9 - emptyCount;

    // mofclient.c：依難度設定門檻，「忙碌數低於門檻且尚未累計」就開始計時。
    bool needStartTimer = false;
    switch (m_difficulty)
    {
        case 0:
            if (busyCount < 5 && !m_isAccumulating)
                needStartTimer = true;
            break;
        case 1:
            if (busyCount < 7 && !m_isAccumulating)
                needStartTimer = true;
            break;
        case 2:
            if (busyCount < 9 && !m_isAccumulating)
                needStartTimer = true;
            break;
        default:
            break;
    }

    if (needStartTimer)
    {
        m_timer.InitTimer(1);
        m_isAccumulating = 1;
    }

    if (m_isAccumulating != 1)
        return;

    // mofclient.c：難度 0 → 滿 4 秒一次補 ≤2 人；難度 1 → 滿 3 秒；難度 2 → 滿 2 秒。
    switch (m_difficulty)
    {
        case 0:
        {
            if (static_cast<int>(m_timer.GetCurrentSecond()) < 4)
                return;
            for (int i = 0; i < 2; ++i)
            {
                AutoPatientRecall();
                if (++busyCount >= 5)
                    break;
            }
            m_isAccumulating = 0;
            break;
        }
        case 1:
        {
            if (static_cast<int>(m_timer.GetCurrentSecond()) < 3)
                return;
            int loops = 0;
            while (true)
            {
                AutoPatientRecall();
                if (++busyCount >= 7)
                {
                    m_isAccumulating = 0;
                    break;
                }
                if (++loops >= 2)
                {
                    m_isAccumulating = 0;
                    return;
                }
            }
            break;
        }
        case 2:
        {
            if (static_cast<int>(m_timer.GetCurrentSecond()) < 2)
                return;
            int loops = 0;
            while (true)
            {
                AutoPatientRecall();
                if (++busyCount >= 9)
                {
                    m_isAccumulating = 0;
                    break;
                }
                if (++loops >= 2)
                {
                    m_isAccumulating = 0;
                    return;
                }
            }
            break;
        }
        default:
            break;
    }
}
