#include "Effect/CEffectBase.h"

// 假設的全域變數，用於 IsCliping 函式
// 這些變數應在遊戲主系統中定義和更新
extern struct GameSystemInfo {
    int ScreenWidth;
    int ScreenHeight;
} g_Game_System_Info;


// 對應反組譯碼: 0x0053A2C0
CEffectBase::CEffectBase()
{
    // m_ccaEffect 會在此處自動呼叫其建構函式，對應原始碼中的：
    // CCAEffect::CCAEffect((CEffectBase *)((char *)this + 36));

    m_pOwner = nullptr;          // *((_DWORD *)this + 1) = 0;
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fDirectionX = 0.0f;
    m_fDirectionY = 0.0f;
    m_fSpeed = 0.0f;             // *((_DWORD *)this + 6) = 0;
    m_bIsFlip = false;           // *((_BYTE *)this + 28) = 0;
    m_bIsVisible = false;
}

// 對應反組譯碼: 0x0053A300
CEffectBase::~CEffectBase()
{
    // *(_DWORD *)this = &CEffectBase::`vftable'; // 由編譯器處理
    // m_ccaEffect 的解構函式會在此處自動被呼叫，對應原始碼中的：
    // CCAEffect::~CCAEffect((CEffectBase *)((char *)this + 36));
}

/**
 * @brief 處理特效的生命週期。
 * 這是基底類別的預設行為，直接返回 false，表示特效永不結束。
 * 衍生類別必須覆寫此函式以提供自己的結束條件。
 */
 // 對應反組譯碼: 0x0053A310
bool CEffectBase::FrameProcess(float fElapsedTime)
{
    // 預設情況下，特效不會自動結束。
    return false; // return 0;
}

/**
 * @brief 檢查特效是否在螢幕的可視範圍內，加上一個緩衝區。
 * 這是為了優化效能，避免處理和繪製完全在螢幕外的特效。
 */
 // 對應反組譯碼: 0x0053A340
bool CEffectBase::IsCliping(float x, float y)
{
    // 原始碼中使用 150.0 作為螢幕外的緩衝區
    const float fBuffer = 150.0f;

    // 檢查 X 座標是否在 [ -緩衝區, 螢幕寬度 + 緩衝區 ] 的範圍內
    if (x + fBuffer < 0.0f) {
        return false;
    }
    if (x > static_cast<float>(g_Game_System_Info.ScreenWidth) + fBuffer) {
        return false;
    }

    // 原始碼沒有檢查 Y 座標，但一個完整的實作通常會包含它
    // if (y + fBuffer < 0.0f) return FALSE;
    // if (y > static_cast<float>(g_Game_System_Info.ScreenHeight) + fBuffer) return FALSE;

    return true;
}