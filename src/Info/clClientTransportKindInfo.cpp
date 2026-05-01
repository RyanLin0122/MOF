// =============================================================================
// clClientTransportKindInfo  —  客戶端載具資訊（含動畫快取）
// 對齊反編譯：mofclient.c 195094 (ctor) ~ 195219 (GetTransportAniInfoDown)
// =============================================================================
#include "Info/clClientTransportKindInfo.h"
#include "Info/clTransportAniInfo.h"

#include <cstdio>
#include <cstring>
#include <windows.h>

clClientTransportKindInfo::clClientTransportKindInfo()
    : clTransportKindInfo()
{
    // mofclient.c 195094：ctor 呼叫 base ctor 後設定 vftable。
    // 兩條 cache 陣列由 BSS 預設 0；明確清零以維持「全 nullptr」前提。
    std::memset(m_pAniInfoUp,   0, sizeof(m_pAniInfoUp));
    std::memset(m_pAniInfoDown, 0, sizeof(m_pAniInfoDown));
}

clClientTransportKindInfo::~clClientTransportKindInfo()
{
    // mofclient.c 195103：scalar deleting destructor 呼叫 ~clClientTransportKindInfo，
    // 但反編譯體本身只還原 vftable；cache 釋放由外部呼叫 Free() 觸發，不在 dtor 內。
}

// -----------------------------------------------------------------------------
// 釋放兩個 cache 陣列上所有非 nullptr 的 clTransportAniInfo。
// 對齊 mofclient.c 195112：
//   v1  = (this + 262152)  /  v2 = 0xFFFF（65535 次 iteration）
//   每輪：先 free (v1 - 0xFFFF dwords) = up[i]，再 free *v1 = down[i]
//        然後 ++v1, --v2
// 結果：清掉 up[0..65534] 與 down[0..65534]（最後一個 [65535] 被忽略）。
// -----------------------------------------------------------------------------
void clClientTransportKindInfo::Free()
{
    // 嚴格對齊原始 binary：只清前 65535 個（不清 [65535]）。
    constexpr std::size_t kCount = 0xFFFF;
    for (std::size_t i = 0; i < kCount; ++i)
    {
        if (m_pAniInfoUp[i])
        {
            delete m_pAniInfoUp[i];
            m_pAniInfoUp[i] = nullptr;
        }
        if (m_pAniInfoDown[i])
        {
            delete m_pAniInfoDown[i];
            m_pAniInfoDown[i] = nullptr;
        }
    }
}

// -----------------------------------------------------------------------------
// 取得「上半身」動畫資訊
// 對齊 mofclient.c 195140：
//   v2 = (char*)this + 4*a2 + 12;     (= &m_pAniInfoUp[a2])
//   if (cache) return cache;
//   info = clTransportKindInfo::GetTransportKindInfo(this, a2);
//   if (!info) return nullptr;
//   if (stricmp(info->aniFileUp, "NONE") == 0) return nullptr;
//   ani = new clTransportAniInfo();  cache = ani;
//   if (!ani->Init(info->aniFileUp)) { MessageBox; return nullptr; }
//   return cache;
// -----------------------------------------------------------------------------
clTransportAniInfo* clClientTransportKindInfo::GetTransportAniInfoUp(std::uint16_t a2)
{
    clTransportAniInfo*& cacheSlot = m_pAniInfoUp[a2];
    if (cacheSlot)
        return cacheSlot;

    stTransportKindInfo* info = GetTransportKindInfo(a2);
    if (!info)
        return nullptr;

    char* v4 = info->aniFileUp;
    if (_stricmp(v4, "NONE") == 0)
        return nullptr;

    clTransportAniInfo* ani = new clTransportAniInfo();
    cacheSlot = ani;

    if (!ani->Init(v4))
    {
        char Text[1024];
        wsprintfA(Text, "Transport animation file was a failure to initialize. [ %s ]", v4);
        MessageBoxA(nullptr, Text, "", 0);
        return nullptr;
    }
    return cacheSlot;
}

// -----------------------------------------------------------------------------
// 取得「下半身」動畫資訊（同上，但 offset = 38；cache 為 m_pAniInfoDown）
// 對齊 mofclient.c 195181。
// -----------------------------------------------------------------------------
clTransportAniInfo* clClientTransportKindInfo::GetTransportAniInfoDown(std::uint16_t a2)
{
    clTransportAniInfo*& cacheSlot = m_pAniInfoDown[a2];
    if (cacheSlot)
        return cacheSlot;

    stTransportKindInfo* info = GetTransportKindInfo(a2);
    if (!info)
        return nullptr;

    char* v4 = info->aniFileDown;
    if (_stricmp(v4, "NONE") == 0)
        return nullptr;

    clTransportAniInfo* ani = new clTransportAniInfo();
    cacheSlot = ani;

    if (!ani->Init(v4))
    {
        char Text[1024];
        wsprintfA(Text, "Transport animation file was a failure to initialize. [ %s ]", v4);
        MessageBoxA(nullptr, Text, "", 0);
        return nullptr;
    }
    return cacheSlot;
}
